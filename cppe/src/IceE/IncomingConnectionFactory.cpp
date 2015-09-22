// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IncomingConnectionFactory.h>
#include <IceE/ConnectionI.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Properties.h>
#include <IceE/Transceiver.h>
#include <IceE/Acceptor.h>
#include <IceE/ThreadPool.h>
#include <IceE/Endpoint.h>
#include <IceE/LocalException.h>
#include <IceE/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(IncomingConnectionFactory* p) { return p; }

void
IceInternal::IncomingConnectionFactory::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    list<ConnectionIPtr> connections;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // First we wait until the connection factory itself is in holding
        // state.
        //
        while(_state < StateHolding)
        {
            wait();
        }

        //
        // We want to wait until all connections are in holding state
        // outside the thread synchronization.
        //
        connections = _connections;
    }

    //
    // Now we wait until each connection is in holding state.
    //
    for_each(connections.begin(), connections.end(), Ice::constVoidMemFun(&ConnectionI::waitUntilHolding));
}

void
IceInternal::IncomingConnectionFactory::waitUntilFinished()
{
    list<ConnectionIPtr> connections;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // First we wait until the factory is destroyed. If we are using
        // an acceptor, we also wait for it to be closed.
        //
        while(_state != StateClosed || _acceptor)
        {
            wait();
        }

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = 0;

        //
        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    for_each(connections.begin(), connections.end(), Ice::voidMemFun(&ConnectionI::waitUntilFinished));

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _connections.clear();
    }
}

EndpointPtr
IceInternal::IncomingConnectionFactory::endpoint() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint;
}

#ifdef ICEE_HAS_BATCH
void
IceInternal::IncomingConnectionFactory::flushBatchRequests()
{
    list<ConnectionIPtr> c;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        c = _connections;
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

bool
IceInternal::IncomingConnectionFactory::readable() const
{
    return false;
}

bool
IceInternal::IncomingConnectionFactory::read(BasicStream&)
{
    assert(false); // Must not be called, readable() returns false.
    return false;
}

class PromoteFollower
{
public:

    PromoteFollower(const ThreadPoolPtr& threadPool) :
        _threadPool(threadPool)
    {
    }

    ~PromoteFollower()
    {
        _threadPool->promoteFollower();
    }

private:

    const ThreadPoolPtr _threadPool;
};

void
IceInternal::IncomingConnectionFactory::message(BasicStream&, const ThreadPoolPtr& threadPool)
{
    ConnectionIPtr connection;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // This makes sure that we promote a follower before we leave
        // the scope of the mutex above, but after we call accept()
        // (if we call it).
        //
        // If _threadPool is null, then this class doesn't do
        // anything.
        //
        PromoteFollower promote(threadPool);

        if(_state != StateActive)
        {
            IceUtil::ThreadControl::yield();
            return;
        }
        
        //
        // Reap connections for which destruction has completed.
        //
        _connections.erase(remove_if(_connections.begin(), _connections.end(),
                                     Ice::constMemFun(&ConnectionI::isFinished)),
                           _connections.end());
        
        //
        // Now accept a new connection.
        //
        TransceiverPtr transceiver;
        try
        {
            transceiver = _acceptor->accept();
        }
        catch(const SocketException& ex)
        {
            if(noMoreFds(ex.error))
            {
                Error out(_instance->initializationData().logger);
                out << "fatal error: can't accept more connections:\n" << ex.toString() << '\n' 
                    << _acceptor->toString();
#ifdef _WIN32_WCE
                exit(1);
#else
                abort();
#endif
            }

            // Ignore socket exceptions.
            return;
        }
        catch(const TimeoutException&)
        {
            // Ignore timeouts.
            return;
        }
        catch(const LocalException& ex)
        {
            // Warn about other Ice local exceptions.
            if(_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "connection exception:\n" << ex.toString() << '\n' << _acceptor->toString();
            }
            return;
        }

        assert(transceiver);

        try
        {
            connection = new ConnectionI(_instance, transceiver, _endpoint, _adapter);
        }
        catch(const LocalException& ex)
        {
            try
            {
                transceiver->close();
            }
            catch(const Ice::LocalException&)
            {
                // Ignore.
            }

            if(_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "connection exception:\n" << ex.toString() << '\n' << _acceptor->toString();
            }
            return;
        }

        _connections.push_back(connection);
    }

    assert(connection);

    connection->start(this);
}

void
IceInternal::IncomingConnectionFactory::finished(const ThreadPoolPtr& threadPool)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    threadPool->promoteFollower();
    assert(threadPool.get() == _instance->serverThreadPool().get());
    assert(_state == StateClosed);

    _instance->serverThreadPool()->decFdsInUse();
    _acceptor->close();
    _acceptor = 0;
    _fd = 0;
    notifyAll();
}

void
IceInternal::IncomingConnectionFactory::exception(const LocalException&)
{
    assert(false); // Must not be called.
}

string
IceInternal::IncomingConnectionFactory::toString() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_acceptor);
    return _acceptor->toString();
}

void
IceInternal::IncomingConnectionFactory::connectionStartCompleted(const Ice::ConnectionIPtr& connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Initialy, connections are in the holding state. If the factory is active
    // we activate the connection.
    //
    if(_state == StateActive)
    {
        connection->activate();
    }
}

void
IceInternal::IncomingConnectionFactory::connectionStartFailed(const Ice::ConnectionIPtr& connection,
                                                              const Ice::LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state == StateClosed)
    {
        return;
    }
    
    if(_warn)
    {
        Warning out(_instance->initializationData().logger);
        out << "connection exception:\n" << ex.toString() << '\n' << _acceptor->toString();
    }
        
    //
    // If the connection is finished, remove it right away from
    // the connection map. Otherwise, we keep it in the map, it
    // will eventually be reaped.
    //
    if(connection->isFinished())
    {
        _connections.remove(connection);
    }
}

IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
                                                                  const EndpointPtr& endpoint,
                                                                  const ObjectAdapterPtr& adapter) :
    EventHandler(instance),
    _endpoint(endpoint),
    _adapter(adapter),
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _state(StateHolding)
{
    if(_instance->defaultsAndOverrides()->overrideTimeout)
    {
        const_cast<EndpointPtr&>(_endpoint) =
            _endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
    }

    _acceptor = _endpoint->acceptor(const_cast<EndpointPtr&>(_endpoint));
    assert(_acceptor);
    _acceptor->listen();
    _fd = _acceptor->fd();

    try
    {
        _instance->serverThreadPool()->incFdsInUse();
    }
    catch(const IceUtil::Exception&)
    {
        try
        {
            _acceptor->close();
        }
        catch(const LocalException&)
        {
            // Here we ignore any exceptions in close().
        }
        _acceptor = 0;
        throw;
    }
}

IceInternal::IncomingConnectionFactory::~IncomingConnectionFactory()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    assert(_state == StateClosed);
    assert(!_acceptor);
    assert(_connections.empty());
}

void
IceInternal::IncomingConnectionFactory::setState(State state)
{
    if(_state == state) // Don't switch twice.
    {
        return;
    }

    switch(state)
    {
        case StateActive:
        {
            if(_state != StateHolding) // Can only switch from holding to active.
            {
                return;
            }
            _instance->serverThreadPool()->_register(this);
            for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&ConnectionI::activate));
            break;
        }
        
        case StateHolding:
        {
            if(_state != StateActive) // Can only switch from active to holding.
            {
                return;
            }
            _instance->serverThreadPool()->unregister(this);
            for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&ConnectionI::hold));
            break;
        }
        
        case StateClosed:
        {
            _instance->serverThreadPool()->finish(this);
#ifdef _STLP_BEGIN_NAMESPACE
            // voidbind2nd is an STLport extension for broken compilers in IceE/Functional.h
            for_each(_connections.begin(), _connections.end(),
                     voidbind2nd(Ice::voidMemFun1(&ConnectionI::destroy), ConnectionI::ObjectAdapterDeactivated));
#else
            for_each(_connections.begin(), _connections.end(),
                     bind2nd(Ice::voidMemFun1(&ConnectionI::destroy), ConnectionI::ObjectAdapterDeactivated));
#endif
            break;
        }
    }

    _state = state;
    notifyAll();
}
