// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ConnectRequestHandler.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/Proxy.h>
#include <IceE/ConnectionI.h>
#include <IceE/RouterInfo.h>
#include <IceE/LocatorInfo.h>
#include <IceE/Outgoing.h>
#include <IceE/OutgoingAsync.h>
#include <IceE/Protocol.h>
#include <IceE/Properties.h>
#include <IceE/ThreadPool.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

#if defined(ICEE_HAS_AMI) || defined(ICEE_HAS_BATCH)
namespace
{

class FlushRequestsWithException : public ThreadPoolWorkItem
{
public:

    FlushRequestsWithException(const ConnectRequestHandlerPtr& handler, const Ice::LocalException& ex) :
        _handler(handler),
        _exception(dynamic_cast<Ice::LocalException*>(ex.ice_clone()))
    {
    }

    virtual void
    execute(const ThreadPoolPtr& threadPool)
    {
        threadPool->promoteFollower();
        _handler->flushRequestsWithException(*_exception.get());
    }

private:

    const ConnectRequestHandlerPtr _handler;
    const auto_ptr<Ice::LocalException> _exception;
};

class FlushRequestsWithExceptionWrapper : public ThreadPoolWorkItem
{
public:

    FlushRequestsWithExceptionWrapper(const ConnectRequestHandlerPtr& handler, const LocalExceptionWrapper& ex) :
        _handler(handler),
        _exception(ex)
    {
    }

    virtual void
    execute(const ThreadPoolPtr& threadPool)
    {
        threadPool->promoteFollower();
        _handler->flushRequestsWithException(_exception);
    }

private:

    const ConnectRequestHandlerPtr _handler;
    const LocalExceptionWrapper _exception;
};

#ifdef ICEE_HAS_AMI
class FlushSentRequests : public ThreadPoolWorkItem
{
public:

    FlushSentRequests(const InstancePtr& instance, const vector<OutgoingAsyncMessageCallbackPtr>& callbacks) :
        _instance(instance), _callbacks(callbacks)
    {
    }

    virtual void
    execute(const ThreadPoolPtr& threadPool)
    {
        threadPool->promoteFollower();
        for(vector<OutgoingAsyncMessageCallbackPtr>::const_iterator p = _callbacks.begin(); p != _callbacks.end(); ++p)
        {
            (*p)->__sentCallback(_instance);
        }
    }

private:

    InstancePtr _instance;
    vector<OutgoingAsyncMessageCallbackPtr> _callbacks;
};
#endif

};

ConnectRequestHandler::ConnectRequestHandler(const ReferencePtr& ref, const Ice::ObjectPrx& proxy) :
    _reference(ref),
    _proxy(proxy),
#if defined(ICEE_HAS_AMI) && defined(ICEE_HAS_LOCATOR)
    _locatorInfoEndpoints(false),
    _locatorInfoCachedEndpoints(false),
#endif
    _response(ref->getMode() == ReferenceModeTwoway),
#ifdef ICEE_HAS_BATCH
    _batchAutoFlush(
        ref->getInstance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
#endif
    _initialized(false),
    _flushing(false),
#if !defined(ICEE_HAS_AMI)
    _connect(false),
#endif
#ifdef ICEE_HAS_BATCH
    _batchRequestInProgress(false),
    _batchRequestsSize(sizeof(requestBatchHdr)),
    _batchStream(ref->getInstance().get(), ref->getInstance()->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                 ref->getInstance()->initializationData().stringConverter,
                 ref->getInstance()->initializationData().wstringConverter,
#endif
                 _batchAutoFlush),
#endif
    _updateRequestHandler(false)
{
}

ConnectRequestHandler::~ConnectRequestHandler()
{
}

RequestHandlerPtr
ConnectRequestHandler::connect()
{
#if !defined(ICEE_HAS_AMI)
    //
    // If there's no AMI support, we can't connect here as this would
    // cause batch requests to block. Instead, the connection will be
    // established on the first synchronous call on the proxy (or when
    // the proxy batch requests are flushed).
    // 
    Lock sync(*this);
    _connect = true;
    _updateRequestHandler = true; // The proxy request handler will be updated when the connection is set.
    return this;
#else
    _reference->getConnection(this);

    Lock sync(*this);
    if(initialized())
    {
        assert(_connection);
        return _connection;
    }
    else
    {
        _updateRequestHandler = true; // The proxy request handler will be updated when the connection is set.
        return this;
    }
#endif
}

#ifdef ICEE_HAS_BATCH
void
ConnectRequestHandler::prepareBatchRequest(BasicStream* os)
{
    {
        Lock sync(*this);
        while(_batchRequestInProgress)
        {
            wait();
        }

        if(!initialized())
        {
            _batchRequestInProgress = true;
            _batchStream.swap(*os);
            return;
        }
    }
    _connection->prepareBatchRequest(os);
}

void
ConnectRequestHandler::finishBatchRequest(BasicStream* os)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            assert(_batchRequestInProgress);
            _batchRequestInProgress = false;
            notifyAll();

            _batchStream.swap(*os);

            if(!_batchAutoFlush &&
               _batchStream.b.size() + _batchRequestsSize > _reference->getInstance()->messageSizeMax())
            {
                Ice::throwMemoryLimitException(__FILE__, __LINE__);
            }

            _batchRequestsSize += _batchStream.b.size();

            Request req;
            req.os = new BasicStream(_reference->getInstance().get(), _reference->getInstance()->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                                     _reference->getInstance()->initializationData().stringConverter,
                                     _reference->getInstance()->initializationData().wstringConverter,
#endif
                                     _batchAutoFlush);
            req.os->swap(_batchStream);
            _requests.push_back(req);
            return;
        }
    }
    _connection->finishBatchRequest(os);
}

void
ConnectRequestHandler::abortBatchRequest()
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            assert(_batchRequestInProgress);
            _batchRequestInProgress = false;
            notifyAll();

            BasicStream dummy(_reference->getInstance().get(), _reference->getInstance()->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                              _reference->getInstance()->initializationData().stringConverter,
                              _reference->getInstance()->initializationData().wstringConverter,
#endif
                              _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestsSize = sizeof(requestBatchHdr);

            return;
        }
    }
    _connection->abortBatchRequest();
}
#endif

Ice::ConnectionI*
ConnectRequestHandler::sendRequest(Outgoing* out, bool /*response*/)
{
    return getConnection(true)->sendRequest(out, _response);
}

#ifdef ICEE_HAS_AMI
bool
ConnectRequestHandler::sendAsyncRequest(const OutgoingAsyncPtr& out, bool /*response*/)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            Request req;
            req.out = out;
            _requests.push_back(req);
            return false;
        }
    }
    return _connection->sendAsyncRequest(out, _response);
}
#endif

#ifdef ICEE_HAS_BATCH
bool
ConnectRequestHandler::flushBatchRequests(BatchOutgoing* out)
{
    return getConnection(true)->flushBatchRequests(out);
}

#ifdef ICEE_HAS_AMI
bool
ConnectRequestHandler::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& out)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            Request req;
            req.batchOut = out;
            _requests.push_back(req);
            return false;
        }
    }
    return _connection->flushAsyncBatchRequests(out);
}
#endif
#endif

Ice::ConnectionIPtr
ConnectRequestHandler::getConnection(bool waitInit)
{
    if(waitInit)
    {
        //
        // Wait for the connection establishment to complete or fail.
        //
        Lock sync(*this);
#if !defined(ICEE_HAS_AMI)
        if(_connect)
        {
            //
            // If there's no AMI support, we establish the connection synchronously
            // from this waiting user thread.
            //
            _connect = false;
            sync.release();
            try
            {
                setConnection(_reference->getConnection());
            }
            catch(const Ice::LocalException& ex)
            {
                setException(ex);
            }
            sync.acquire();
        }
#endif
        while(!_initialized && !_exception.get())
        {
            wait();
        }
    }

    if(_exception.get())
    {
        _exception->ice_throw();
        return false; // Keep the compiler happy.
    }
    else
    {
        assert(!waitInit || _initialized);
        return _connection;
    }
}

void
ConnectRequestHandler::setConnection(const Ice::ConnectionIPtr& connection)
{
    {
        Lock sync(*this);
        assert(!_exception.get() && !_connection);
        assert(_updateRequestHandler || _requests.empty());
        _connection = connection;
    }

#if defined(ICEE_HAS_ROUTER)
    RouterInfoPtr ri = _reference->getRouterInfo();
#ifndef ICEE_PURE_CLIENT 
    //
    // If we have a router, set the object adapter for this router
    // (if any) to the new connection, so that callbacks from the
    // router can be received over this new connection.
    //
    if(ri && ri->getAdapter())
    {
        connection->setAdapter(ri->getAdapter());
    }
#endif

    //
    // If this proxy is for a non-local object, and we are using a router, then
    // add this proxy to the router info object.
    //
#ifdef ICEE_HAS_AMI
    if(ri && !ri->addProxy(_proxy, this))
    {
        return; // The request handler will be initialized once addProxy returns.
    }
#else
    if(ri)
    {
        ri->addProxy(_proxy);
    }
#endif

#endif

    //
    // We can now send the queued requests.
    //
    flushRequests();
}

void
ConnectRequestHandler::setException(const Ice::LocalException& ex)
{
#if defined(ICEE_HAS_AMI) && defined(ICEE_HAS_LOCATOR)
    if(_locatorInfoEndpoints && !dynamic_cast<const Ice::NoEndpointException*>(&ex))
    {
        assert(_reference->getLocatorInfo());
        _reference->getLocatorInfo()->clearCache(_reference);

        if(_locatorInfoCachedEndpoints)
        {
            TraceLevelsPtr traceLevels = _reference->getInstance()->traceLevels();
            if(traceLevels->retry >= 2)
            {
                Trace out(_reference->getInstance()->initializationData().logger, traceLevels->retryCat);
                out << "connection to cached endpoints failed\n"
                    << "removing endpoints from cache and trying one more time\n" << ex.toString();
            }
            
            //
            // Reset locator info flags and retry.
            //
            _locatorInfoEndpoints = false;
            _locatorInfoCachedEndpoints = false;
            dynamic_cast<RoutableReference*>(_reference.get())->getConnectionNoRouterInfo(this);
            return;
        }
    }
#endif

    Lock sync(*this);
    assert(!_initialized && !_exception.get());
    assert(_updateRequestHandler || _requests.empty());
    _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
    _proxy = 0; // Break cyclic reference count.

    //
    // If some requests were queued, we notify them of the failure. This is done from a thread
    // from the client thread pool since this will result in ice_exception callbacks to be
    // called.
    //
    if(!_requests.empty())
    {
        _reference->getInstance()->clientThreadPool()->execute(new FlushRequestsWithException(this, ex));
    }

    notifyAll();
}

void
ConnectRequestHandler::flushRequestsWithException(const Ice::LocalException& ex)
{
    for(deque<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
#ifdef ICEE_HAS_AMI
        if(p->out)
        {
            p->out->__finished(ex);
        }
#ifdef ICEE_HAS_BATCH
        else if(p->batchOut)
        {
            p->batchOut->__finished(ex);
        }
        else
#endif
#endif
#ifdef ICEE_HAS_BATCH
        {
            assert(p->os);
            delete p->os;
        }
#endif
    }
    _requests.clear();
}

void
ConnectRequestHandler::flushRequestsWithException(const LocalExceptionWrapper& ex)
{
    for(deque<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
#ifdef ICEE_HAS_AMI
        if(p->out)
        {
            p->out->__finished(ex);
        }
#ifdef ICEE_HAS_BATCH
        else if(p->batchOut)
        {
            p->batchOut->__finished(*ex.get());
        }
        else
#endif
#endif
#ifdef ICEE_HAS_BATCH
        {
            assert(p->os);
            delete p->os;
        }
#endif
    }
    _requests.clear();
}

#if defined(ICEE_HAS_AMI) && defined(ICEE_HAS_ROUTER)
void
ConnectRequestHandler::routerInfoEndpoints(const vector<EndpointPtr>& endpoints)
{
    vector<EndpointPtr> endpts = endpoints;
    if(!endpts.empty())
    {
        RoutableReferencePtr ref = RoutableReferencePtr::dynamicCast(_reference);
        assert(ref);
        ref->applyOverrides(endpts);
        ref->createConnection(endpts, this);
        return;
    }

    dynamic_cast<RoutableReference*>(_reference.get())->getConnectionNoRouterInfo(this);
}

void
ConnectRequestHandler::routerInfoAddedProxy()
{
    //
    // The proxy was added to the router info, we're now ready to send the
    // queued requests.
    //
    flushRequests();
}
#endif

#if defined(ICEE_HAS_AMI) && defined(ICEE_HAS_LOCATOR)
void
ConnectRequestHandler::locatorInfoEndpoints(const vector<EndpointPtr>& endpoints, bool cached)
{
    //
    // We set the following flags to ensure setException will try with fresh
    // endpoints if the endpoints are cached.
    //
    _locatorInfoEndpoints = true;
    _locatorInfoCachedEndpoints = cached;

    if(endpoints.empty())
    {
        setException(Ice::NoEndpointException(__FILE__, __LINE__, _reference->toString()));
        return;
    }

    RoutableReferencePtr ref = RoutableReferencePtr::dynamicCast(_reference);
    assert(ref);

    vector<EndpointPtr> endpts = endpoints;
    ref->applyOverrides(endpts);
    ref->createConnection(endpts, this);
}
#endif

bool
ConnectRequestHandler::initialized()
{
    // Must be called with the mutex locked.

    if(_initialized)
    {
        assert(_connection);
        return true;
    }
    else
    {
        while(_flushing && !_exception.get())
        {
            wait();
        }

        if(_exception.get())
        {
            _exception->ice_throw();
            return false; // Keep the compiler happy.
        }
        else
        {
            return _initialized;
        }
    }
}

void
ConnectRequestHandler::flushRequests()
{
    {
        Lock sync(*this);
        assert(_connection && !_initialized);

#ifdef ICEE_HAS_BATCH
        while(_batchRequestInProgress)
        {
            wait();
        }
#endif

        //
        // We set the _flushing flag to true to prevent any additional queuing. Callers
        // might block for a little while as the queued requests are being sent but this
        // shouldn't be an issue as the request sends are non-blocking.
        //
        _flushing = true;
    }


#ifdef ICEE_HAS_AMI
    vector<OutgoingAsyncMessageCallbackPtr> sentCallbacks;
#endif
    try
    {
        while(!_requests.empty()) // _requests is immutable when _flushing = true
        {
            Request& req = _requests.front();
#ifdef ICEE_HAS_AMI
            if(req.out)
            {
                if(_connection->sendAsyncRequest(req.out, _response))
                {
                    if(dynamic_cast<Ice::AMISentCallback*>(req.out.get()))
                    {
                        sentCallbacks.push_back(req.out);
                    }
                }
            }
#ifdef ICEE_HAS_BATCH
            else if(req.batchOut)
            {
                if(_connection->flushAsyncBatchRequests(req.batchOut))
                {
                    if(dynamic_cast<Ice::AMISentCallback*>(req.batchOut.get()))
                    {
                        sentCallbacks.push_back(req.batchOut);
                    }
                }
            }
            else
#endif
#endif
#ifdef ICEE_HAS_BATCH
            {
                BasicStream os(req.os->instance(), req.os->instance()->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                                  , req.os->instance()->initializationData().stringConverter,
                                  req.os->instance()->initializationData().wstringConverter
#endif
                              );
                _connection->prepareBatchRequest(&os);
                try
                {
                    const Ice::Byte* bytes;
                    req.os->i = req.os->b.begin();
                    req.os->readBlob(bytes, req.os->b.size());
                    os.writeBlob(bytes, req.os->b.size());
                    _connection->finishBatchRequest(&os);
                    delete req.os;
                }
                catch(const Ice::LocalException&)
                {
                    _connection->abortBatchRequest();
                    throw;
                }
            }
#endif
            _requests.pop_front();
        }
    }
    catch(const LocalExceptionWrapper& ex)
    {
        Lock sync(*this);
        assert(!_exception.get() && !_requests.empty());
        _exception.reset(dynamic_cast<Ice::LocalException*>(ex.get()->ice_clone()));
        _reference->getInstance()->clientThreadPool()->execute(new FlushRequestsWithExceptionWrapper(this, ex));
    }
    catch(const Ice::LocalException& ex)
    {
        Lock sync(*this);
        assert(!_exception.get() && !_requests.empty());
        _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
        _reference->getInstance()->clientThreadPool()->execute(new FlushRequestsWithException(this, ex));
    }

#ifdef ICEE_HAS_AMI
    if(!sentCallbacks.empty())
    {
        InstancePtr instance = _reference->getInstance();
        instance->clientThreadPool()->execute(new FlushSentRequests(instance, sentCallbacks));
    }
#endif

    //
    // We've finished sending the queued requests and the request handler now send
    // the requests over the connection directly. It's time to substitute the
    // request handler of the proxy with the more efficient connection request
    // handler which does not have any synchronization. This also breaks the cyclic
    // reference count with the proxy.
    //
    if(_updateRequestHandler && !_exception.get())
    {
        _proxy->__setRequestHandler(_connection, this);
    }

    {
        Lock sync(*this);
        assert(!_initialized);
        if(!_exception.get())
        {
            _initialized = true;
            _flushing = false;
        }
        _proxy = 0; // Break cyclic reference count.
        notifyAll();
    }
}

#endif
