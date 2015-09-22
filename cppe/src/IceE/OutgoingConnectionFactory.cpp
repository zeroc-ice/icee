// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/ConnectionI.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Transceiver.h>
#include <IceE/Connector.h>
#if defined(ICEE_HAS_ROUTER) && !defined(ICEE_PURE_CLIENT)
#   include <IceE/RouterInfo.h>
#endif
#include <IceE/LocalException.h>
#include <IceE/Functional.h>
#include <IceE/Random.h>

#include <list>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(OutgoingConnectionFactory* p) { return p; }

namespace
{
struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
        return IceUtilInternal::random(static_cast<int>(d));
    }
};
}

bool
IceInternal::OutgoingConnectionFactory::ConnectorInfo::operator<(const ConnectorInfo& other) const
{
    return connector < other.connector;
}

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        return;
    }

#ifdef _STLP_BEGIN_NAMESPACE
    // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
    for_each(_connections.begin(), _connections.end(),
             voidbind2nd(Ice::secondVoidMemFun1<ConnectorInfo, ConnectionI, ConnectionI::DestructionReason>
                         (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));
#else
    for_each(_connections.begin(), _connections.end(),
             bind2nd(Ice::secondVoidMemFun1<const ConnectorInfo, ConnectionI, ConnectionI::DestructionReason>
                     (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));
#endif

    _destroyed = true;
    notifyAll();
}

void
IceInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    multimap<ConnectorInfo, ConnectionIPtr> connections;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // First we wait until the factory is destroyed. We also wait
        // until there are no pending connections anymore. Only then
        // we can be sure the _connections contains all connections.
        //
        while(!_destroyed || !_pending.empty() || _pendingConnectCount > 0)
        {
            wait();
        }

        //
        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    for_each(connections.begin(), connections.end(),
             Ice::secondVoidMemFun<const ConnectorInfo, ConnectionI>(&ConnectionI::waitUntilFinished));

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _connections.clear();
        _connectionsByEndpoint.clear();
    }
}

#ifndef ICEE_HAS_AMI
Ice::ConnectionIPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpts)
{
    assert(!endpts.empty());
    
    //
    // Apply the overrides.
    //
    vector<EndpointPtr> endpoints = applyOverrides(endpts);

    //
    // Try to find a connection to one of the given endpoints.
    // 
    Ice::ConnectionIPtr connection = findConnection(endpoints);
    if(connection)
    {
        return connection;
    }

    auto_ptr<Ice::LocalException> exception;

    //
    // If we didn't find a connection with the endpoints, we create the connectors
    // for the endpoints.
    //
    vector<ConnectorInfo> connectors;
    for(vector<EndpointPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        //
        // Create connectors for the endpoint.
        //
        try
        {
            vector<ConnectorPtr> cons = (*p)->connectors();
            assert(!cons.empty());
            for(vector<ConnectorPtr>::const_iterator r = cons.begin(); r != cons.end(); ++r)
            {
                assert(*r);
                connectors.push_back(ConnectorInfo(*r, *p));
            }
        }
        catch(const Ice::LocalException& ex)
        {
            exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            handleException(ex, p != endpoints.end() - 1);
        }
    }

    if(connectors.empty())
    {
        assert(exception.get());
        exception->ice_throw();
    }
    
    //
    // Try to get a connection to one of the connectors. A null result indicates that no
    // connection was found and that we should try to establish the connection (and that
    // the connectors were added to _pending to prevent other threads from establishing
    // the connection).
    //
    connection = getConnection(connectors);
    if(connection)
    {
        return connection;
    }

    //
    // Try to establish the connection to the connectors.
    //
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    for(vector<ConnectorInfo>::const_iterator q = connectors.begin(); q != connectors.end(); ++q)
    {
        try
        {
            connection = createConnection(q->connector->connect(), *q);
#if !defined(ICEE_PURE_CLIENT)
            connection->start(0);
#else
            connection->start();
#endif
            break;
        }
        catch(const Ice::CommunicatorDestroyedException& ex)
        {
            exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            handleException(*exception.get(), *q, connection, q != connectors.end() - 1);
            connection = 0;
            break; // No need to continue
        }
        catch(const Ice::LocalException& ex)
        {
            exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            handleException(*exception.get(), *q, connection, q != connectors.end() - 1);
            connection = 0;
        }
    }

    //
    // Finish creating the connection (this removes the connectors from the _pending
    // list and notifies any waiting threads).
    //
    finishGetConnection(connectors, connection);

    if(!connection)
    {
        assert(exception.get());
        exception->ice_throw();
    }

    return connection;
}
#else
void
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointPtr>& endpts, 
                                               const ConnectRequestHandlerPtr& handler)
{
    assert(!endpts.empty());

    //
    // Apply the overrides.
    //
    vector<EndpointPtr> endpoints = applyOverrides(endpts);

    //
    // Try to find a connection to one of the given endpoints.
    //
    try
    {
        Ice::ConnectionIPtr connection = findConnection(endpoints);
        if(connection)
        {
            handler->setConnection(connection);
            return;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        handler->setException(ex);
        return;
    }

    ConnectCallbackPtr cb = new ConnectCallback(this, endpoints, handler);
    cb->getConnectors();
}
#endif

#if defined(ICEE_HAS_ROUTER) && !defined(ICEE_PURE_CLIENT)
void
IceInternal::OutgoingConnectionFactory::setRouterInfo(const RouterInfoPtr& routerInfo)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(routerInfo);

    //
    // Search for connections to the router's client proxy endpoints,
    // and update the object adapter for such connections, so that
    // callbacks from the router can be received over such
    // connections.
    //
    ObjectAdapterPtr adapter = routerInfo->getAdapter();
    vector<EndpointPtr> endpoints = routerInfo->getClientEndpoints();
    vector<EndpointPtr>::const_iterator p;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        EndpointPtr endpoint = *p;

        //
        // Modify endpoints with overrides.
        //
        if(_instance->defaultsAndOverrides()->overrideTimeout)
        {
            endpoint = endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
        }

        multimap<ConnectorInfo, ConnectionIPtr>::const_iterator q;
        for(q = _connections.begin(); q != _connections.end(); ++q)
        {
            if(q->second->endpoint() == endpoint)
            {
                try
                {
                    q->second->setAdapter(adapter);
                }
                catch(const Ice::LocalException&)
                {
                    //
                    // Ignore, the connection is being closed or closed.
                    //
                }
            }
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::removeAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        return;
    }

    for(multimap<ConnectorInfo, ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
        if(p->second->getAdapter() == adapter)
        {
            try
            {
                p->second->setAdapter(0);
            }
            catch(const Ice::LocalException&)
            {
                //
                // Ignore, the connection is being closed or closed.
                //
            }
        }
    }
}
#endif

#ifdef ICEE_HAS_BATCH
void
IceInternal::OutgoingConnectionFactory::flushBatchRequests()
{
    list<ConnectionIPtr> c;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        for(multimap<ConnectorInfo, ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end();
            ++p)
        {
            c.push_back(p->second);
        }
    }

    for(list<ConnectionIPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
        try
        {
            (*p)->flushBatchRequests();
        }
        catch(const LocalException&)
        {
            // Ignore.
        }
    }
}
#endif

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false),
    _pendingConnectCount(0)
{
}

IceInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(_destroyed);
    assert(_connections.empty());
    assert(_connectionsByEndpoint.empty());
    assert(_pending.empty());
    assert(_pendingConnectCount == 0);
}

vector<EndpointPtr>
IceInternal::OutgoingConnectionFactory::applyOverrides(const vector<EndpointPtr>& endpts)
{
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    vector<EndpointPtr> endpoints = endpts;
    for(vector<EndpointPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        //
        // Modify endpoints with overrides.
        //
        if(defaultsAndOverrides->overrideTimeout)
        {
            *p = (*p)->timeout(defaultsAndOverrides->overrideTimeoutValue);
        }
    }
    return endpoints;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::findConnection(const vector<EndpointPtr>& endpoints)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(!endpoints.empty());
    for(vector<EndpointPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        pair<multimap<EndpointPtr, ConnectionIPtr>::iterator,
             multimap<EndpointPtr, ConnectionIPtr>::iterator> pr = _connectionsByEndpoint.equal_range(*p);

        for(multimap<EndpointPtr, ConnectionIPtr>::iterator q = pr.first; q != pr.second; ++q)
        {
            if(q->second->isActiveOrHolding()) // Don't return destroyed or un-validated connections
            {
                return q->second;
            }
        }
    }
    return 0;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::findConnection(const vector<ConnectorInfo>& connectors)
{
    // This must be called with the mutex locked.

    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        pair<multimap<ConnectorInfo, ConnectionIPtr>::iterator,
             multimap<ConnectorInfo, ConnectionIPtr>::iterator> pr = _connections.equal_range(*p);

        if(pr.first == pr.second)
        {
            continue;
        }

        for(multimap<ConnectorInfo, ConnectionIPtr>::iterator q = pr.first; q != pr.second; ++q)
        {
            if(q->second->isActiveOrHolding()) // Don't return destroyed or un-validated connections
            {
                if(q->second->endpoint() != p->endpoint)
                {
                    _connectionsByEndpoint.insert(pair<const EndpointPtr, ConnectionIPtr>(p->endpoint, q->second));
                }
                return q->second;
            }
        }
    }

    return 0;
}

void
IceInternal::OutgoingConnectionFactory::incPendingConnectCount()
{
    //
    // Keep track of the number of pending connects. The outgoing connection factory
    // waitUntilFinished() method waits for all the pending connects to terminate before
    // to return. This ensures that the communicator client thread pool isn't destroyed
    // too soon and will still be available to execute the ice_exception() callbacks for
    // the asynchronous requests waiting on a connection to be established.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_destroyed)
    {
        throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    ++_pendingConnectCount;
}

void
IceInternal::OutgoingConnectionFactory::decPendingConnectCount()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    --_pendingConnectCount;
    assert(_pendingConnectCount >= 0);
    if(_destroyed && _pendingConnectCount == 0)
    {
        notifyAll();
    }
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::getConnection(const vector<ConnectorInfo>& connectors
#ifdef ICEE_HAS_AMI
                                                      , const ConnectCallbackPtr& cb
#endif
    )
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        //
        // Reap connections for which destruction has completed.
        //
        multimap<ConnectorInfo, ConnectionIPtr>::iterator p = _connections.begin();
        while(p != _connections.end())
        {
            if(p->second->isFinished())
            {
                _connections.erase(p++);
            }
            else
            {
                ++p;
            }
        }

        multimap<EndpointPtr, ConnectionIPtr>::iterator q = _connectionsByEndpoint.begin();
        while(q != _connectionsByEndpoint.end())
        {
            if(q->second->isFinished())
            {
                _connectionsByEndpoint.erase(q++);
            }
            else
            {
                ++q;
            }
        }

        //
        // Try to get the connection. We may need to wait for other threads to
        // finish if one of them is currently establishing a connection to one
        // of our connectors.
        //
        while(!_destroyed)
        {
            //
            // Search for a matching connection. If we find one, we're done.
            //
            Ice::ConnectionIPtr connection = findConnection(connectors);
            if(connection)
            {
#ifdef ICEE_HAS_AMI
                if(cb)
                {
                    //
                    // This might not be the first getConnection call for the callback. We need
                    // to ensure that the callback isn't registered with any other pending
                    // connectors since we just found a connection and therefore don't need to
                    // wait anymore for other pending connectors.
                    //
                    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
                    {
                        map<ConnectorInfo, set<ConnectCallbackPtr> >::iterator q = _pending.find(*p);
                        if(q != _pending.end())
                        {
                            q->second.erase(cb);
                        }
                    }
                }
#endif
                return connection;
            }

            //
            // Determine whether another thread is currently attempting to connect to one of our endpoints;
            // if so we wait until it's done.
            //
            bool found = false;
            for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
            {
#ifndef ICEE_HAS_AMI
                set<ConnectorInfo >::iterator q = _pending.find(*p);
#else
                map<ConnectorInfo, set<ConnectCallbackPtr> >::iterator q = _pending.find(*p);
#endif
                if(q != _pending.end())
                {
                    found = true;
#ifdef ICEE_HAS_AMI
                    if(cb)
                    {
                        q->second.insert(cb); // Add the callback to each pending connector.
                    }
#endif
                }
            }

            if(!found)
            {
                //
                // If no thread is currently establishing a connection to one of our connectors,
                // we get out of this loop and start the connection establishment to one of the
                // given connectors.
                //
                break;
            }
            else
            {
                //
                // If a callback is not specified we wait until another thread notifies us about a
                // change to the pending list. Otherwise, if a callback is provided we're done:
                // when the pending list changes the callback will be notified and will try to
                // get the connection again.
                //
#ifdef ICEE_HAS_AMI
                if(cb)
                {
                    return 0;
                }
#endif
                wait();
            }
        }

        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        //
        // No connection to any of our endpoints exists yet; we add the given connectors to
        // the _pending set to indicate that we're attempting connection establishment to
        // these connectors. We might attempt to connect to the same connector multiple times.
        //
        for(vector<ConnectorInfo>::const_iterator r = connectors.begin(); r != connectors.end(); ++r)
        {
            if(_pending.find(*r) == _pending.end())
            {
#ifndef ICEE_HAS_AMI
                _pending.insert(*r);
#else
                _pending.insert(pair<ConnectorInfo, set<ConnectCallbackPtr> >(*r, set<ConnectCallbackPtr>()));
#endif
            }
        }
    }

#ifdef ICEE_HAS_AMI
    //
    // At this point, we're responsible for establishing the connection to one of
    // the given connectors. If it's a non-blocking connect, calling nextConnector
    // will start the connection establishment. Otherwise, we return null to get
    // the caller to establish the connection.
    //
    if(cb)
    {
        cb->nextConnector();
    }
#endif

    return 0;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::createConnection(const TransceiverPtr& transceiver, const ConnectorInfo& ci)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_pending.find(ci) != _pending.end() && transceiver);

    //
    // Create and add the connection to the connection map. Adding the connection to the map
    // is necessary to support the interruption of the connection initialization and validation
    // in case the communicator is destroyed.
    //
    try
    {
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

#ifndef ICEE_PURE_CLIENT
        Ice::ConnectionIPtr connection = new ConnectionI(_instance, transceiver, ci.endpoint, 0);
#else
        Ice::ConnectionIPtr connection = new ConnectionI(_instance, transceiver, ci.endpoint);
#endif
        _connections.insert(pair<const ConnectorInfo, ConnectionIPtr>(ci, connection));
        _connectionsByEndpoint.insert(pair<const EndpointPtr, ConnectionIPtr>(ci.endpoint, connection));
        return connection;
    }
    catch(const Ice::LocalException&)
    {
        try
        {
            transceiver->close();
        }
        catch(const Ice::LocalException&)
        {
            // Ignore
        }
        throw;
    }
}

void
IceInternal::OutgoingConnectionFactory::finishGetConnection(const vector<ConnectorInfo>& connectors,
                                                            const ConnectionIPtr& connection)
{
#ifdef ICEE_HAS_AMI
    set<ConnectCallbackPtr> callbacks;
#endif

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // We're done trying to connect to the given connectors so we remove the
        // connectors from the pending list and notify waiting threads. We also
        // notify the pending connect callbacks (outside the synchronization).
        //

        for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
        {
#ifndef ICEE_HAS_AMI
            _pending.erase(*p);
#else
            map<ConnectorInfo, set<ConnectCallbackPtr> >::iterator q = _pending.find(*p);
            if(q != _pending.end())
            {
                callbacks.insert(q->second.begin(), q->second.end());
                _pending.erase(q);
            }
#endif
        }
        notifyAll();

        //
        // If the connect attempt succeeded and the communicator is not destroyed,
        // activate the connection!
        //
        if(connection && !_destroyed)
        {
            connection->activate();
        }
    }

#ifdef ICEE_HAS_AMI
    //
    // Notify any waiting callbacks.
    //
    for(set<ConnectCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        (*p)->getConnection();
    }
#endif
}

void
IceInternal::OutgoingConnectionFactory::handleException(const LocalException& ex, const ConnectorInfo& ci,
                                                        const ConnectionIPtr& connection, bool hasMore)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->retry >= 2)
    {
        Trace out(_instance->initializationData().logger, traceLevels->retryCat);

        out << "connection to endpoint failed";
        if(dynamic_cast<const CommunicatorDestroyedException*>(&ex))
        {
            out << "\n";
        }
        else
        {
            if(hasMore)
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
        }
        out << ex.toString();
    }

    if(connection && connection->isFinished())
    {
        //
        // If the connection is finished, we remove it right away instead of
        // waiting for the reaping.
        //
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            pair<multimap<ConnectorInfo, ConnectionIPtr>::iterator,
                 multimap<ConnectorInfo, ConnectionIPtr>::iterator> pr = _connections.equal_range(ci);

            for(multimap<ConnectorInfo, ConnectionIPtr>::iterator p = pr.first; p != pr.second; ++p)
            {
                if(p->second == connection)
                {
                    _connections.erase(p);
                    break;
                }
            }

            pair<multimap<EndpointPtr, ConnectionIPtr>::iterator,
                 multimap<EndpointPtr, ConnectionIPtr>::iterator> qr = _connectionsByEndpoint.equal_range(ci.endpoint);

            for(multimap<EndpointPtr, ConnectionIPtr>::iterator q = qr.first; q != qr.second; ++q)
            {
                if(q->second == connection)
                {
                    _connectionsByEndpoint.erase(q);
                    break;
                }
            }
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::handleException(const LocalException& ex, bool hasMore)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->retry >= 2)
    {
        Trace out(_instance->initializationData().logger, traceLevels->retryCat);

        out << "couldn't resolve endpoint host";
        if(dynamic_cast<const CommunicatorDestroyedException*>(&ex))
        {
            out << "\n";
        }
        else
        {
            if(hasMore)
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
        }
        out << ex.toString();
    }
}

#ifdef ICEE_HAS_AMI
IceInternal::OutgoingConnectionFactory::ConnectCallback::ConnectCallback(const OutgoingConnectionFactoryPtr& factory,
                                                                         const vector<EndpointPtr>& endpoints,
                                                                         const ConnectRequestHandlerPtr& handler) :
    _factory(factory),
    _endpoints(endpoints),
    _handler(handler)
{
    _endpointsIter = _endpoints.begin();
}

//
// Methods from ConnectionI::StartCallback
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartCompleted(const ConnectionIPtr& connection)
{
    _factory->finishGetConnection(_connectors, connection);
    _handler->setConnection(connection);
    _factory->decPendingConnectCount(); // Must be called last.
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartFailed(const ConnectionIPtr& connection,
                                                                               const LocalException& ex)
{
    assert(_iter != _connectors.end());

    _factory->handleException(ex, *_iter, connection, _iter != _connectors.end() - 1);
    if(dynamic_cast<const Ice::CommunicatorDestroyedException*>(&ex)) // No need to continue.
    {
        _factory->finishGetConnection(_connectors, 0);
        _handler->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
    else if(++_iter != _connectors.end()) // Try the next connector.
    {
        nextConnector();
    }
    else
    {
        _factory->finishGetConnection(_connectors, 0);
        _handler->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

//
// Methods from Endpoint_connectors
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectors(const vector<ConnectorPtr>& connectors)
{
    vector<ConnectorPtr> cons = connectors;

    RandomNumberGenerator rng;
    random_shuffle(cons.begin(), cons.end(), rng);

    for(vector<ConnectorPtr>::const_iterator p = cons.begin(); p != cons.end(); ++p)
    {
        _connectors.push_back(ConnectorInfo(*p, *_endpointsIter));
    }

    if(++_endpointsIter != _endpoints.end())
    {
        nextEndpoint();
    }
    else
    {
        assert(!_connectors.empty());

        //
        // We now have all the connectors for the given endpoints. We can try to obtain the
        // connection.
        //
        _iter = _connectors.begin();
        getConnection();
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::exception(const Ice::LocalException& ex)
{
    _factory->handleException(ex, _endpointsIter != _endpoints.end() - 1);
    if(++_endpointsIter != _endpoints.end())
    {
        nextEndpoint();
    }
    else if(!_connectors.empty())
    {
        //
        // We now have all the connectors for the given endpoints. We can try to obtain the
        // connection.
        //
        _iter = _connectors.begin();
        getConnection();
    }
    else
    {
        _handler->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::getConnectors()
{
    try
    {
        //
        // Notify the factory that there's an async connect pending. This is necessary
        // to prevent the outgoing connection factory to be destroyed before all the
        // pending asynchronous connects are finished.
        //
        _factory->incPendingConnectCount();
    }
    catch(const Ice::LocalException& ex)
    {
        _handler->setException(ex);
        return;
    }

    nextEndpoint();
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::nextEndpoint()
{
    try
    {
        assert(_endpointsIter != _endpoints.end());
        (*_endpointsIter)->connectors_async(this);
    }
    catch(const Ice::LocalException& ex)
    {
        exception(ex);
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::getConnection()
{
    try
    {
        //
        // If all the connectors have been created, we ask the factory to get a
        // connection.
        //
        Ice::ConnectionIPtr connection = _factory->getConnection(_connectors, this);
        if(!connection)
        {
            //
            // A null return value from getConnection indicates that the connection
            // is being established and that everthing has been done to ensure that
            // the callback will be notified when the connection establishment is
            // done.
            //
            return;
        }

        _handler->setConnection(connection);
        _factory->decPendingConnectCount(); // Must be called last.
    }
    catch(const Ice::LocalException& ex)
    {
        _handler->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::nextConnector()
{
    Ice::ConnectionIPtr connection;
    try
    {
        assert(_iter != _connectors.end());
        connection = _factory->createConnection(_iter->connector->connect(), *_iter);
        connection->start(this);
    }
    catch(const Ice::LocalException& ex)
    {
        connectionStartFailed(connection, ex);
    }
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::operator<(const ConnectCallback& rhs) const
{
    return this < &rhs;
}

#endif
