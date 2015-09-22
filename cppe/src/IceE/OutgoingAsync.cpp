// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/OutgoingAsync.h>
#include <IceE/ConnectionI.h>
#include <IceE/RequestHandler.h>
#include <IceE/Reference.h>
#include <IceE/Instance.h>
#include <IceE/LocalException.h>
#include <IceE/Properties.h>
#include <IceE/LoggerUtil.h>
#include <IceE/LocatorInfo.h>
#include <IceE/ProxyFactory.h>
#include <IceE/RouterInfo.h>
#include <IceE/Outgoing.h> // For LocalExceptionWrapper.
#include <IceE/Protocol.h>
#include <IceE/ReplyStatus.h>
#include <IceE/ThreadPool.h>
#include <IceE/RetryQueue.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifdef ICEE_HAS_AMI

IceUtil::Shared* IceInternal::upCast(OutgoingAsyncMessageCallback* p) { return p; }
IceUtil::Shared* IceInternal::upCast(OutgoingAsync* p) { return p; }
#ifdef ICEE_HAS_BATCH
IceUtil::Shared* IceInternal::upCast(BatchOutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(AMI_Object_ice_flushBatchRequests* p) { return p; }
#endif

namespace
{

class CallException : public ThreadPoolWorkItem
{
public:

    CallException(const OutgoingAsyncMessageCallbackPtr& outAsync, const Ice::LocalException& ex) :
        _outAsync(outAsync), _exception(dynamic_cast<Ice::LocalException*>(ex.ice_clone()))
    {
    }

    virtual void
    execute(const ThreadPoolPtr& threadPool)
    {
        threadPool->promoteFollower();
        _outAsync->__exception(*_exception.get());
    }

private:

    const OutgoingAsyncMessageCallbackPtr _outAsync;
    const auto_ptr<Ice::LocalException> _exception;
};

};

IceInternal::OutgoingAsyncMessageCallback::OutgoingAsyncMessageCallback() :
    __is(0),
    __os(0)
{
}

IceInternal::OutgoingAsyncMessageCallback::~OutgoingAsyncMessageCallback()
{
    assert(!__is);
    assert(!__os);
}

void
IceInternal::OutgoingAsyncMessageCallback::__sentCallback(const InstancePtr& instance)
{
    try
    {
        dynamic_cast<Ice::AMISentCallback*>(this)->ice_sent();
    }
    catch(const std::exception& ex)
    {
        __warning(instance, ex);
    }
    catch(...)
    {
        __warning(instance);
    }
}

void
IceInternal::OutgoingAsyncMessageCallback::__exception(const Ice::Exception& exc)
{
    try
    {
        ice_exception(exc);
    }
    catch(const std::exception& ex)
    {
        __warning(ex);
    }
    catch(...)
    {
        __warning();
    }

    __releaseCallback();
}

void
IceInternal::OutgoingAsyncMessageCallback::__acquireCallback(const Ice::ObjectPrx& proxy)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(__monitor);

    //
    // We must first wait for other requests to finish.
    //
    while(__os)
    {
        __monitor.wait();
    }

    Instance* instance = proxy->__reference()->getInstance().get();
    assert(!__os);
    __os = new BasicStream(instance, instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                           , instance->initializationData().stringConverter,
                           instance->initializationData().wstringConverter
#endif
                          );
}

void
IceInternal::OutgoingAsyncMessageCallback::__releaseCallback(const Ice::LocalException& exc)
{
    assert(__os);

    //
    // This is called by the invoking thread to release the callback following a direct
    // failure to marshal/send the request. We call the ice_exception() callback with
    // the thread pool to avoid potential deadlocks in case the invoking thread locked
    // some mutexes/resources (which couldn't be re-acquired by the callback).
    //

    try
    {
        __os->instance()->clientThreadPool()->execute(new CallException(this, exc));
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        __releaseCallback();
        throw; // CommunicatorDestroyedException is the only exception that can propagate directly.
    }
}

void
IceInternal::OutgoingAsyncMessageCallback::__releaseCallbackNoSync()
{
    if(__is)
    {
        delete __is;
        __is = 0;
    }

    assert(__os);
    delete __os;
    __os = 0;

    __monitor.notify();
}

void
IceInternal::OutgoingAsyncMessageCallback::__warning(const std::exception& exc) const
{
    if(__os) // Don't print anything if release() was already called.
    {
        __warning(__os->instance());
    }
}

void
IceInternal::OutgoingAsyncMessageCallback::__warning(const InstancePtr& instance, const std::exception& exc) const
{
    if(instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
        Warning out(instance->initializationData().logger);
        const Exception* ex = dynamic_cast<const Exception*>(&exc);
        if(ex)
        {
            out << "Ice::Exception raised by AMI callback:\n" << ex->toString();
        }
        else
        {
            out << "std::exception raised by AMI callback:\n" << exc.what();
        }
    }
}

void
IceInternal::OutgoingAsyncMessageCallback::__warning() const
{
    if(__os) // Don't print anything if release() was already called.
    {
        __warning(__os->instance());
    }
}

void
IceInternal::OutgoingAsyncMessageCallback::__warning(const InstancePtr& instance) const
{
    if(instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
        Warning out(instance->initializationData().logger);
        out << "unknown exception raised by AMI callback";
    }
}

void
IceInternal::OutgoingAsync::__sent(Ice::ConnectionI* connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(__monitor);
    _sent = true;

    if(!_proxy->ice_isTwoway())
    {
        __releaseCallbackNoSync(); // No response expected, we're done with the OutgoingAsync.
    }
    else if(_response)
    {
        __monitor.notifyAll(); // If the response was already received notify finished() which is waiting.
    }
    else if(connection->timeout() > 0)
    {
        assert(!_timerTaskConnection && __os);
        _timerTaskConnection = connection;
        IceUtil::Time timeout = IceUtil::Time::milliSeconds(connection->timeout());
        __os->instance()->timer()->schedule(this, timeout);
    }
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    assert(_proxy->ice_isTwoway()); // Can only be called for twoways.

    Ice::Byte replyStatus;
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(__monitor);
        assert(__os);
        _response = true;

        if(_timerTaskConnection && __os->instance()->timer()->cancel(this))
        {
            _timerTaskConnection = 0; // Timer cancelled.
        }

        while(!_sent || _timerTaskConnection)
        {
            __monitor.wait();
        }

        __is = new BasicStream(__os->instance(), __os->instance()->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                               , __os->instance()->initializationData().stringConverter,
                               __os->instance()->initializationData().wstringConverter
#endif
                              );
        __is->swap(is);
        __is->read(replyStatus);

        switch(replyStatus)
        {
            case replyOK:
            case replyUserException:
            {
                break;
            }

            case replyObjectNotExist:
            case replyFacetNotExist:
            case replyOperationNotExist:
            {
                Identity ident;
                ident.__read(__is);

                //
                // For compatibility with the old FacetPath.
                //
                vector<string> facetPath;
                __is->read(facetPath);
                string facet;
                if(!facetPath.empty())
                {
                    if(facetPath.size() > 1)
                    {
                        throw MarshalException(__FILE__, __LINE__);
                    }
                    facet.swap(facetPath[0]);
                }

                string operation;
                __is->read(operation, false);

                auto_ptr<RequestFailedException> ex;
                switch(replyStatus)
                {
                    case replyObjectNotExist:
                    {
                        ex.reset(new ObjectNotExistException(__FILE__, __LINE__));
                        break;
                    }

                    case replyFacetNotExist:
                    {
                        ex.reset(new FacetNotExistException(__FILE__, __LINE__));
                        break;
                    }

                    case replyOperationNotExist:
                    {
                        ex.reset(new OperationNotExistException(__FILE__, __LINE__));
                        break;
                    }

                    default:
                    {
                        assert(false);
                        break;
                    }
                }

                ex->id = ident;
                ex->facet = facet;
                ex->operation = operation;
                ex->ice_throw();
            }

            case replyUnknownException:
            case replyUnknownLocalException:
            case replyUnknownUserException:
            {
                string unknown;
                __is->read(unknown, false);

                auto_ptr<UnknownException> ex;
                switch(replyStatus)
                {
                    case replyUnknownException:
                    {
                        ex.reset(new UnknownException(__FILE__, __LINE__));
                        break;
                    }

                    case replyUnknownLocalException:
                    {
                        ex.reset(new UnknownLocalException(__FILE__, __LINE__));
                        break;
                    }

                    case replyUnknownUserException:
                    {
                        ex.reset(new UnknownUserException(__FILE__, __LINE__));
                        break;
                    }

                    default:
                    {
                        assert(false);
                        break;
                    }
                }

                ex->unknown = unknown;
                ex->ice_throw();
            }

            default:
            {
                //throw UnknownReplyStatusException(__FILE__, __LINE__);
                throw ProtocolException(__FILE__, __LINE__, "unknown reply status");
            }
        }
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }

    assert(replyStatus == replyOK || replyStatus == replyUserException);

    try
    {
        __is->startReadEncaps();
        __response(replyStatus == replyOK);
    }
    catch(const std::exception& ex)
    {
        __warning(ex);
        __releaseCallback();
    }
    catch(...)
    {
        __warning();
        __releaseCallback();
    }
}

void
IceInternal::OutgoingAsync::__finished(const Ice::LocalException& exc)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(__monitor);
        assert(__os);

        if(_timerTaskConnection && __os->instance()->timer()->cancel(this))
        {
            _timerTaskConnection = 0; // Timer cancelled.
        }

        while(_timerTaskConnection)
        {
            __monitor.wait();
        }
    }

    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback.
    //

    try
    {
        handleException(exc); // This will throw if the invocation can't be retried.
    }
    catch(const Ice::LocalException& ex)
    {
        __exception(ex);
    }
}

void
IceInternal::OutgoingAsync::__finished(const LocalExceptionWrapper& ex)
{
    assert(__os && !_sent);

    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback. The LocalExceptionWrapper exception is only called
    // before the invocation is sent.
    //

    try
    {
        handleException(ex); // This will throw if the invocation can't be retried.
    }
    catch(const Ice::LocalException& ex)
    {
        __exception(ex);
    }
}

void
IceInternal::OutgoingAsync::__retry(int interval)
{
    //
    // This method is called by the proxy to retry an invocation, no
    // other threads can access this object.
    //
    if(interval > 0)
    {
        assert(__os);
        __os->instance()->retryQueue()->add(this, interval);
    }
    else
    {
        __send();
    }
}

bool
IceInternal::OutgoingAsync::__send()
{
    try
    {
        _sent = false;
        _response = false;
        _handler = _proxy->__getRequestHandler();
        _sentSynchronously = _handler->sendAsyncRequest(this, _proxy->ice_isTwoway());
    }
    catch(const LocalExceptionWrapper& ex)
    {
        handleException(ex);
    }
    catch(const Ice::LocalException& ex)
    {
        handleException(ex);
    }
    return _sentSynchronously;
}

void
IceInternal::OutgoingAsync::__prepare(const ObjectPrx& prx, const string& operation, OperationMode mode,
                                      const Context* context)
{
    _proxy = prx;
    _handler = 0;
    _cnt = 0;
    _mode = mode;

    //
    // Can't call async via a batch proxy.
    //
    if(_proxy->ice_isBatchOneway() || _proxy->ice_isBatchDatagram())
    {
        throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "can't send batch requests with AMI");
    }
    else if(_proxy->ice_isDatagram())
    {
        throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "datagram proxy");
    }

    __os->writeBlob(requestHdr, sizeof(requestHdr));

    Reference* ref = _proxy->__reference().get();

    ref->getIdentity().__write(__os);

    //
    // For compatibility with the old FacetPath.
    //
    if(ref->getFacet().empty())
    {
        __os->write(static_cast<string*>(0), static_cast<string*>(0));
    }
    else
    {
        string facet = ref->getFacet();
        __os->write(&facet, &facet + 1);
    }

    __os->write(operation, false);

    __os->write(static_cast<Byte>(_mode));

    if(context == 0)
    {
        context = ref->getContext();
    }

    __os->writeSize(Int(context->size()));
    Context::const_iterator p;
    for(p = context->begin(); p != context->end(); ++p)
    {
        __os->write(p->first);
        __os->write(p->second);
    }

    __os->startWriteEncaps();
}

void
IceInternal::OutgoingAsync::handleException(const LocalExceptionWrapper& ex)
{
    if(_mode == Nonmutating || _mode == Idempotent)
    {
        _proxy->__handleExceptionWrapperRelaxed(_handler, ex, this, _cnt);
    }
    else
    {
        _proxy->__handleExceptionWrapper(_handler, ex, this);
    }
}

void
IceInternal::OutgoingAsync::handleException(const Ice::LocalException& exc)
{
    try
    {
        //
        // A CloseConnectionException indicates graceful server shutdown, and is therefore
        // always repeatable without violating "at-most-once". That's because by sending a
        // close connection message, the server guarantees that all outstanding requests
        // can safely be repeated.
        //
        // An ObjectNotExistException can always be retried as well without violating
        // "at-most-once" (see the implementation of the checkRetryAfterException method of
        // the ProxyFactory class for the reasons why it can be useful).
        //
        if(!_sent ||
           dynamic_cast<const CloseConnectionException*>(&exc) ||
           dynamic_cast<const ObjectNotExistException*>(&exc))
        {
            exc.ice_throw();
        }

        //
        // Throw the exception wrapped in a LocalExceptionWrapper, to indicate that the
        // request cannot be resent without potentially violating the "at-most-once"
        // principle.
        //
        throw LocalExceptionWrapper(exc, false);
    }
    catch(const LocalExceptionWrapper& ex)
    {
        if(_mode == Nonmutating || _mode == Idempotent)
        {
            _proxy->__handleExceptionWrapperRelaxed(_handler, ex, this, _cnt);
        }
        else
        {
            _proxy->__handleExceptionWrapper(_handler, ex, this);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        _proxy->__handleException(_handler, ex, this, _cnt);
    }
}

void
IceInternal::OutgoingAsync::runTimerTask() // Implementation of TimerTask::runTimerTask()
{
    Ice::ConnectionIPtr connection;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(__monitor);
        assert(_timerTaskConnection && _sent); // Can only be set once the request is sent.

        if(!_response) // If the response was just received, don't close the connection.
        {
            connection = _timerTaskConnection;
        }
        _timerTaskConnection = 0;
        __monitor.notifyAll();
    }

    if(connection)
    {
        connection->exception(Ice::TimeoutException(__FILE__, __LINE__));
    }
}

#ifdef ICEE_HAS_BATCH

void
IceInternal::BatchOutgoingAsync::__sent(Ice::ConnectionI* /*connection*/)
{
    __releaseCallback();
}

void
IceInternal::BatchOutgoingAsync::__finished(const Ice::LocalException& exc)
{
    __exception(exc);
}

bool
Ice::AMI_Object_ice_flushBatchRequests::__invoke(const ObjectPrx& prx)
{
    __acquireCallback(prx);
    try
    {
        //
        // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
        // requests were queued with the connection, they would be lost without being noticed.
        //
        RequestHandlerPtr handler;
        int cnt = -1; // Don't retry.
        try
        {
            handler = prx->__getRequestHandler();
            return handler->flushAsyncBatchRequests(this);
        }
        catch(const Ice::LocalException& ex)
        {
            prx->__handleException(handler, ex, 0, cnt);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        __releaseCallback(ex);
    }
    return false;
}

#endif
#endif
