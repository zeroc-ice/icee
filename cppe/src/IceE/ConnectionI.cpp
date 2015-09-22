// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/ConnectionI.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Properties.h>
#include <IceE/TraceUtil.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Transceiver.h>
#include <IceE/ThreadPool.h>
#include <IceE/SelectorThread.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapter.h> // For getThreadPool() and getServantManager().
#endif
#include <IceE/Endpoint.h>
#include <IceE/Outgoing.h>
#include <IceE/OutgoingAsync.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/Incoming.h>
#endif
#include <IceE/LocalException.h>
#include <IceE/ReferenceFactory.h> // For createProxy().
#include <IceE/ProxyFactory.h> // For createProxy().

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Connection* p) { return p; }
IceUtil::Shared* IceInternal::upCast(ConnectionI* p) { return p; }

namespace IceInternal
{

#ifdef ICEE_HAS_AMI
class FlushSentCallbacks : public ThreadPoolWorkItem
{
public:

    FlushSentCallbacks(const Ice::ConnectionIPtr& connection) : _connection(connection)
    {
    }

    void
    execute(const ThreadPoolPtr& threadPool)
    {
        threadPool->promoteFollower();
        _connection->flushSentCallbacks();
    }

private:

    const Ice::ConnectionIPtr _connection;
};

#endif

}

void
Ice::ConnectionI::OutgoingMessage::adopt(BasicStream* str)
{
    if(adopted)
    {
        if(str)
        {
            delete stream;
            stream = 0;
            adopted = false;
        }
        else
        {
            return; // Stream is already adopted.
        }
    }
    else if(!str)
    {
#ifdef ICEE_HAS_AMI
        if(out || outAsync)
#else
        if(out)
#endif
        {
            return; // Adopting request stream is not necessary.
        }
        else
        {
            str = stream; // Adopt this stream
            stream = 0;
        }
    }

    assert(str);
    stream = new BasicStream(str->instance(), str->instance()->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                             , str->instance()->initializationData().stringConverter,
                             str->instance()->initializationData().wstringConverter
#endif
                            );

    stream->swap(*str);
    adopted = true;
}

void
Ice::ConnectionI::OutgoingMessage::sent(ConnectionI* connection, bool notify)
{
    if(out)
    {
        out->sent(notify); // true = notify the waiting thread that the request was sent.
    }
#ifdef ICEE_HAS_AMI
    else if(outAsync)
    {
        outAsync->__sent(connection);
    }
#endif

    if(adopted)
    {
        delete stream;
        stream = 0;
    }
}

void
Ice::ConnectionI::OutgoingMessage::finished(const Ice::LocalException& ex)
{
    if(!response)
    {
        //
        // Only notify oneway requests. The connection keeps track of twoway
        // requests in the _requests/_asyncRequests maps and will notify them
        // of the connection exceptions.
        //
        if(out)
        {
            out->finished(ex);
        }
#ifdef ICEE_HAS_AMI
        else if(outAsync)
        {
            outAsync->__finished(ex);
        }
#endif
    }

    if(adopted)
    {
        delete stream;
        stream = 0;
    }
}

void
#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
Ice::ConnectionI::start(const StartCallbackPtr& callback)
#else
Ice::ConnectionI::start()
#endif
{
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_state == StateClosed) // The connection might already be closed if the communicator was destroyed.
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        SocketStatus status = initialize();
        if(status == Finished)
        {
            status = validate();
        }

        if(status != Finished)
        {
            int timeout;
            DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
            if(defaultsAndOverrides->overrideConnectTimeout)
            {
                timeout = defaultsAndOverrides->overrideConnectTimeoutValue;
            }
            else
            {
                timeout = _endpoint->timeout();
            }

            _sendInProgress = true;
            _selectorThread->_register(_transceiver->fd(), this, status, timeout);

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
            if(callback)
            {
                _startCallback = callback;
                return;
            }
#endif

            //
            // Wait for the connection to be validated.
            //
            while(_state <= StateNotValidated)
            {
                wait();
            }

            if(_state >= StateClosing)
            {
                assert(_exception.get());
                _exception->ice_throw();
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        exception(ex);
#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
        if(callback)
        {
            callback->connectionStartFailed(this, *_exception.get());
            return;
        }
        else
#endif
        {
            waitUntilFinished();
            throw;

        }
    }

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    if(callback)
    {
        callback->connectionStartCompleted(this);
    }
#endif
}

void
Ice::ConnectionI::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        return;
    }

    setState(StateActive);
}

#ifndef ICEE_PURE_CLIENT
void
Ice::ConnectionI::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        return;
    }

    setState(StateHolding);
}
#endif

void
Ice::ConnectionI::destroy(DestructionReason reason)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    switch(reason)
    {
#ifndef ICEE_PURE_CLIENT
        case ObjectAdapterDeactivated:
        {
            setState(StateClosing, ObjectAdapterDeactivatedException(__FILE__, __LINE__));
            break;
        }
#endif

        case CommunicatorDestroyed:
        {
            setState(StateClosing, CommunicatorDestroyedException(__FILE__, __LINE__));
            break;
        }
    }
}

void
Ice::ConnectionI::close(bool force)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(force)
    {
        setState(StateClosed, ForcedCloseConnectionException(__FILE__, __LINE__));
    }
    else
    {
        //
        // If we do a graceful shutdown, then we wait until all
        // outstanding requests have been completed. Otherwise, the
        // CloseConnectionException will cause all outstanding
        // requests to be retried, regardless of whether the server
        // has processed them or not.
        //
#ifdef ICEE_HAS_AMI
        while(!_requests.empty() || !_asyncRequests.empty())
#else
        while(!_requests.empty())
#endif
        {
            wait();
        }

        setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

bool
Ice::ConnectionI::isActiveOrHolding() const
{
    //
    // We can not use trylock here, otherwise the outgoing connection
    // factory might return destroyed (closing or closed) connections,
    // resulting in connection retry exhaustion.
    //
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    return _state > StateNotValidated && _state < StateClosing;
}

bool
Ice::ConnectionI::isFinished() const
{
    //
    // We can use trylock here, because as long as there are still
    // threads operating in this connection object, connection
    // destruction is considered as not yet finished.
    //
    IceUtil::Monitor<IceUtil::Mutex>::TryLock sync(*this);

    if(!sync.acquired())
    {
        return false;
    }

#ifndef ICEE_PURE_CLIENT
    if(_transceiver || _dispatchCount != 0)
#else
    if(_transceiver)
#endif
    {
        return false;
    }

    assert(_state == StateClosed);
    return true;
}

void
Ice::ConnectionI::throwException() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_exception.get())
    {
        assert(_state >= StateClosing);
        _exception->ice_throw();
    }
}

#ifndef ICEE_PURE_CLIENT
void
Ice::ConnectionI::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(_state < StateHolding || _dispatchCount > 0)
    {
        wait();
    }
}
#endif

void
Ice::ConnectionI::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // We wait indefinitely until connection closing has been
    // initiated. We also wait indefinitely until all outstanding
    // requests are completed. Otherwise we couldn't guarantee
    // that there are no outstanding calls when deactivate() is
    // called on the servant locators.
    //
#ifndef ICEE_PURE_CLIENT
    while(_state < StateClosing || _dispatchCount > 0)
#else
    while(_state < StateClosing)
#endif
    {
        wait();
    }

    //
    // Now we must wait until close() has been called on the
    // transceiver.
    //
    while(_transceiver)
    {
        if(_state != StateClosed && _endpoint->timeout() >= 0)
        {
            IceUtil::Time timeout = IceUtil::Time::milliSeconds(_endpoint->timeout());
            IceUtil::Time waitTime = _stateTime + timeout - IceUtil::Time::now(IceUtil::Time::Monotonic);

            if(waitTime > IceUtil::Time())
            {
                //
                // We must wait a bit longer until we close this
                // connection.
                //
                if(!timedWait(waitTime))
                {
                    setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
                }
            }
            else
            {
                //
                // We already waited long enough, so let's close this
                // connection!
                //
                setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
            }

            //
            // No return here, we must still wait until close() is
            // called on the _transceiver.
            //
        }
        else
        {
            wait();
        }
    }

    assert(_state == StateClosed);

#ifndef ICEE_PURE_CLIENT
    //
    // Clear the OA. See bug 1673 for the details of why this is necessary.
    //
    _adapter = 0;
#endif
}

#ifdef ICEE_HAS_BATCH
void
Ice::ConnectionI::flushBatchRequests()
{
    BatchOutgoing out(this, _instance.get());
    out.invoke();
}
#endif

#ifndef ICEE_PURE_CLIENT

void
Ice::ConnectionI::sendResponse(BasicStream* os)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            notifyAll();
        }

        if(_state == StateClosed)
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        OutgoingMessage message(os);
        sendMessage(message);

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

void
Ice::ConnectionI::sendNoResponse()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            notifyAll();
        }

        if(_state == StateClosed)
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

#endif

EndpointPtr
Ice::ConnectionI::endpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

#ifndef ICEE_PURE_CLIENT

void
Ice::ConnectionI::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_state == StateClosing || _state == StateClosed)
    {
        assert(_exception.get());
        _exception->ice_throw();
    }
    else if(_state <= StateNotValidated)
    {
        return;
    }

    _adapter = adapter;

    if(_adapter)
    {
        _servantManager = adapter->getServantManager();
        if(!_servantManager)
        {
            _adapter = 0;
        }
    }
    else
    {
        _servantManager = 0;
    }

    //
    // We never change the thread pool with which we were initially
    // registered, even if we add or remove an object adapter.
    //
}

ObjectAdapterPtr
Ice::ConnectionI::getAdapter() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _adapter;
}

ObjectPrx
Ice::ConnectionI::createProxy(const Identity& ident) const
{
    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    ConnectionIPtr self = const_cast<ConnectionI*>(this);
    return _instance->proxyFactory()->referenceToProxy(_instance->referenceFactory()->create(ident, self));
}

#endif

ConnectionI*
Ice::ConnectionI::sendRequest(Outgoing* out, bool response)
{
    BasicStream* os = out->os();

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_exception.get())
    {
        //
        // If the connection is closed before we even have a chance
        // to send our request, we always try to send the request
        // again.
        //
        throw LocalExceptionWrapper(*_exception.get(), true);
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    Int requestId;
    if(response)
    {
        //
        // Create a new unique request ID.
        //
        requestId = _nextRequestId++;
        if(requestId <= 0)
        {
            _nextRequestId = 1;
            requestId = _nextRequestId++;
        }

        //
        // Fill in the request ID.
        //
        const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }

    //
    // Send the message. If it can't be sent without blocking the message is added
    // to _sendStreams and it will be sent by the selector thread.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(out, os, response);
        sent = sendMessage(message);
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    if(response)
    {
        //
        // Add to the requests map.
        //
        _requestsHint = _requests.insert(_requests.end(), pair<const Int, Outgoing*>(requestId, out));
    }

    return sent && !response ? 0 : this;
}

#ifdef ICEE_HAS_AMI
bool
Ice::ConnectionI::sendAsyncRequest(const OutgoingAsyncPtr& out, bool response)
{
    BasicStream* os = out->__getOs();

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_exception.get())
    {
        //
        // If the exception is closed before we even have a chance
        // to send our request, we always try to send the request
        // again.
        //
        throw LocalExceptionWrapper(*_exception.get(), true);
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    Int requestId;
    if(response)
    {
        //
        // Create a new unique request ID.
        //
        requestId = _nextRequestId++;
        if(requestId <= 0)
        {
            _nextRequestId = 1;
            requestId = _nextRequestId++;
        }

        //
        // Fill in the request ID.
        //
        const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }

    bool sent = false;
    try
    {
        OutgoingMessage message(out, os, response);
        sent = sendMessage(message);
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    if(response)
    {
        //
        // Add to the async requests map.
        //
        _asyncRequestsHint = _asyncRequests.insert(_asyncRequests.end(),
                                                   pair<const Int, OutgoingAsyncPtr>(requestId, out));
    }
    return sent;
}
#endif

#ifdef ICEE_HAS_BATCH

void
Ice::ConnectionI::prepareBatchRequest(BasicStream* os)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Wait if flushing is currently in progress.
    //
    while(_batchStreamInUse && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        _exception->ice_throw();
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    if(_batchStream.b.empty())
    {
        try
        {
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
            ex.ice_throw();
        }
    }

    _batchStreamInUse = true;
    _batchMarker = _batchStream.b.size();
    _batchStream.swap(*os);

    //
    // The batch stream now belongs to the caller, until
    // finishBatchRequest() or abortBatchRequest() is called.
    //
}

void
Ice::ConnectionI::finishBatchRequest(BasicStream* os)
{
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // Get the batch stream back.
        //
        _batchStream.swap(*os);

        if(_exception.get())
        {
            _exception->ice_throw();
        }

        bool flush = false;
        if(_batchAutoFlush)
        {
            //
            // Throw memory limit exception if the first message added causes us to
            // go over limit. Otherwise put aside the marshalled message that caused
            // limit to be exceeded and rollback stream to the marker.
            //
            try
            {
                _transceiver->checkSendSize(_batchStream, _instance->messageSizeMax());
            }
            catch(const Ice::Exception&)
            {
                if(_batchRequestNum > 0)
                {
                    flush = true;
                }
                else
                {
                    throw;
                }
            }
        }

        if(flush)
        {
            //
            // Temporarily save the last request.
            //
            vector<Ice::Byte> lastRequest(_batchStream.b.begin() + _batchMarker, _batchStream.b.end());
            _batchStream.b.resize(_batchMarker);

            //
            // Send the batch stream without the last request.
            //
            try
            {
                //
                // Fill in the number of requests in the batch.
                //
                const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
                reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
                copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif

                OutgoingMessage message(&_batchStream);
                sendMessage(message);
            }
            catch(const Ice::LocalException& ex)
            {
                setState(StateClosed, ex);
                assert(_exception.get());
                _exception->ice_throw();
            }

            //
            // Reset the batch.
            //
            BasicStream dummy(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                              _instance->initializationData().stringConverter,
                              _instance->initializationData().wstringConverter,
#endif
                              _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestNum = 0;
            _batchMarker = 0;

            //
            // Check again if the last request doesn't exceed what we can send with the auto flush
            //
            if(sizeof(requestBatchHdr) + lastRequest.size() >  _instance->messageSizeMax())
            {
                throw MemoryLimitException(__FILE__, __LINE__);
            }

            //
            // Start a new batch with the last message that caused us to go over the limit.
            //
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
            _batchStream.writeBlob(&lastRequest[0], lastRequest.size());
        }

        //
        // Increment the number of requests in the batch.
        //
        ++_batchRequestNum;

        //
        // Notify about the batch stream not being in use anymore.
        //
        assert(_batchStreamInUse);
        _batchStreamInUse = false;
        notifyAll();
    }
    catch(const Ice::LocalException&)
    {
        abortBatchRequest();
        throw;
    }
}

void
Ice::ConnectionI::abortBatchRequest()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    BasicStream dummy(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                      _instance->initializationData().stringConverter,
                      _instance->initializationData().wstringConverter,
#endif
                      _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchMarker = 0;

    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

bool
Ice::ConnectionI::flushBatchRequests(BatchOutgoing* out)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(_batchStreamInUse && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        _exception->ice_throw();
    }

    if(_batchRequestNum == 0)
    {
        out->sent(false);
        return true;
    }

    //
    // Fill in the number of requests in the batch.
    //
    const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif
    _batchStream.swap(*out->os());

    //
    // Send the batch stream.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(out, out->os(), false);
        sent = sendMessage(message);
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    //
    // Reset the batch stream.
    //
    BasicStream dummy(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                      _instance->initializationData().stringConverter,
                      _instance->initializationData().wstringConverter,
#endif
                      _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchMarker = 0;
    return sent;
}

#ifdef ICEE_HAS_AMI
bool
Ice::ConnectionI::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& outAsync)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(_batchStreamInUse && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        _exception->ice_throw();
    }

    if(_batchRequestNum == 0)
    {
        outAsync->__sent(this);
        return true;
    }

    //
    // Fill in the number of requests in the batch.
    //
    const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif
    _batchStream.swap(*outAsync->__getOs());

    //
    // Send the batch stream.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(outAsync, outAsync->__getOs(), false);
        sent = sendMessage(message);
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    //
    // Reset the batch stream.
    //
    BasicStream dummy(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                      _instance->initializationData().stringConverter,
                      _instance->initializationData().wstringConverter,
#endif
                      _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchMarker = 0;
    return sent;
}
#endif

#endif

Ice::ConnectionIPtr
Ice::ConnectionI::getConnection(bool /*wait*/)
{
    return this;
}

bool
Ice::ConnectionI::readable() const
{
    return true;
}

bool
Ice::ConnectionI::read(BasicStream& stream)
{
    return _transceiver->read(stream);
}

void
Ice::ConnectionI::message(BasicStream& stream, const ThreadPoolPtr& threadPool)
{
    Int requestId = 0;
#ifndef ICEE_PURE_CLIENT
    Int invokeNum = 0;
    ServantManagerPtr servantManager;
    ObjectAdapterPtr adapter;
#endif
#ifdef ICEE_HAS_AMI
    OutgoingAsyncPtr outAsync;
#endif

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // We must promote within the synchronization, otherwise there
        // could be various race conditions with close connection
        // messages and other messages.
        //
        threadPool->promoteFollower(this);

        if(_state != StateClosed)
        {
#ifndef ICEE_PURE_CLIENT
#ifdef ICEE_HAS_AMI
            parseMessage(stream, invokeNum, requestId, servantManager, adapter, outAsync);
#else
            parseMessage(stream, invokeNum, requestId, servantManager, adapter);
#endif
#else
#ifdef ICEE_HAS_AMI
            parseMessage(stream, requestId, outAsync);
#else
            parseMessage(stream, requestId);
#endif
#endif
        }

        //
        // parseMessage() can close the connection, so we must check
        // for closed state again.
        //
        if(_state == StateClosed)
        {
            return;
        }
    }

    //
    // Asynchronous replies must be handled outside the thread
    // synchronization, so that nested calls are possible.
    //
#ifdef ICEE_HAS_AMI
    if(outAsync)
    {
        outAsync->__finished(stream);
    }
#endif

#ifndef ICEE_PURE_CLIENT
    //
    // Method invocation (or multiple invocations for batch messages)
    // must be done outside the thread synchronization, so that nested
    // calls are possible.
    //
    invokeAll(stream, invokeNum, requestId, servantManager, adapter);
#endif
}

void
Ice::ConnectionI::finished(const ThreadPoolPtr& threadPool)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(threadPool.get() == _threadPool.get() && _state == StateClosed && !_sendInProgress);

        threadPool->promoteFollower();

        _threadPool->decFdsInUse();
        _selectorThread->decFdsInUse();

#ifdef ICEE_HAS_AMI
        _flushSentCallbacks = 0; // Clear cyclic reference count.
#endif
    }

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    if(_startCallback)
    {
        _startCallback->connectionStartFailed(this, *_exception.get());
        _startCallback = 0;
    }
#endif

    for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        o->finished(*_exception.get());
    }
    _sendStreams.clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage

    for(map<Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        p->second->finished(*_exception.get());
    }
    _requests.clear();

#ifdef ICEE_HAS_AMI
    for(map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
    {
        q->second->__finished(*_exception.get());
    }
    _asyncRequests.clear();
#endif

    //
    // This must be done last as this will cause waitUntilFinished() to return (and communicator
    // objects such as the timer might be destroyed too).
    //
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        try
        {
            _transceiver->close();
            _transceiver = 0;
            notifyAll();
        }
        catch(const Ice::LocalException&)
        {
            _transceiver = 0;
            notifyAll();
            throw;
        }
    }
}

void
Ice::ConnectionI::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

#ifndef ICEE_PURE_CLIENT
void
Ice::ConnectionI::invokeException(const LocalException& ex, int invokeNum)
{
    //
    // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
    // called in case of a fatal exception we decrement _dispatchCount here.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);

    if(invokeNum > 0)
    {
        assert(_dispatchCount > 0);
        _dispatchCount -= invokeNum;
        assert(_dispatchCount >= 0);
        if(_dispatchCount == 0)
        {
            notifyAll();
        }
    }
}
#endif

string
Ice::ConnectionI::type() const
{
    return _type; // No mutex lock, _type is immutable.
}

Ice::Int
Ice::ConnectionI::timeout() const
{
    return _endpoint->timeout(); // No mutex lock, _endpoint is immutable.
}

string
Ice::ConnectionI::toString() const
{
    return _desc; // No mutex lock, _desc is immutable.
}

//
// Operations from SocketReadyCallback
//
SocketStatus
Ice::ConnectionI::socketReady()
{
#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    StartCallbackPtr callback;
#endif
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(_sendInProgress);

        if(_state == StateClosed)
        {
            return Finished;
        }

        try
        {
            //
            // First, we check if there's something to send. If that's the case, the connection
            // must be active and the only thing to do is send the queued streams.
            //
            if(!_sendStreams.empty())
            {
                if(!send())
                {
                    return NeedWrite;
                }
                assert(_sendStreams.empty());
            }
            else
            {
                assert(_state == StateClosed || _state <= StateNotValidated);
                if(_state == StateNotInitialized)
                {
                    SocketStatus status = initialize();
                    if(status != Finished)
                    {
                        return status;
                    }
                }

                if(_state <= StateNotValidated)
                {
                    SocketStatus status = validate();
                    if(status != Finished)
                    {
                        return status;
                    }
                }

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
                swap(_startCallback, callback);
#endif
            }
        }
        catch(const Ice::LocalException& ex)
        {
            setState(StateClosed, ex);
            return Finished;
        }

        assert(_sendStreams.empty());
        _selectorThread->unregister(this);
        _sendInProgress = false;
    }

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    if(callback)
    {
        callback->connectionStartCompleted(this);
    }
#endif
    return Finished;
}

void
Ice::ConnectionI::socketFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_sendInProgress && _state == StateClosed);
    _sendInProgress = false;
    _threadPool->finish(this);
}

void
Ice::ConnectionI::socketTimeout()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        setState(StateClosed, ConnectTimeoutException(__FILE__, __LINE__));
    }
    else if(_state <= StateClosing)
    {
        setState(StateClosed, TimeoutException(__FILE__, __LINE__));
    }
}

Ice::ConnectionI::ConnectionI(const InstancePtr& instance,
                              const TransceiverPtr& transceiver,
                              const EndpointPtr& endpoint
#ifndef ICEE_PURE_CLIENT
                              , const ObjectAdapterPtr& adapter
#endif
                           ) :
    EventHandler(instance, transceiver->fd()),
    _transceiver(transceiver),
    _desc(transceiver->toString()),
    _type(transceiver->type()),
    _endpoint(endpoint),
#ifndef ICEE_PURE_CLIENT
    _adapter(adapter),
#endif
    _logger(_instance->initializationData().logger), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
#ifdef ICEE_HAS_AMI
    _asyncRequestsHint(_asyncRequests.end()),
#endif
#ifdef ICEE_HAS_BATCH
    _batchAutoFlush(
        _instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
    _batchStream(_instance.get(), _instance->messageSizeMax(),
#ifdef ICEE_HAS_WSTRING
                 _instance->initializationData().stringConverter,
                 _instance->initializationData().wstringConverter,
#endif
                 _batchAutoFlush),
    _batchStreamInUse(false),
    _batchRequestNum(0),
    _batchMarker(0),
#endif
    _sendInProgress(false),
#ifndef ICEE_PURE_CLIENT
    _dispatchCount(0),
#endif
    _state(StateNotInitialized),
    _stateTime(IceUtil::Time::now(IceUtil::Time::Monotonic))
{
#ifndef ICEE_PURE_CLIENT
    if(_adapter)
    {
        _servantManager = _adapter->getServantManager();
    }
#endif

    __setNoDelete(true);
    try
    {
/* TODO: Do we have adapter-specific thread pools?
#ifndef ICEE_PURE_CLIENT
        if(_adapter)
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = _adapter->getThreadPool();
        }
        else
#endif
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = _instance->clientThreadPool();
        }
*/
#ifndef ICEE_PURE_CLIENT
        if(_adapter)
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = _instance->serverThreadPool();
        }
        else
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = _instance->clientThreadPool();
        }
#else
        const_cast<ThreadPoolPtr&>(_threadPool) = _instance->clientThreadPool();
#endif
        _threadPool->incFdsInUse();

        const_cast<SelectorThreadPtr&>(_selectorThread) = _instance->selectorThread();
        _selectorThread->incFdsInUse();

#ifdef ICEE_HAS_AMI
        _flushSentCallbacks = new FlushSentCallbacks(this);
#endif
    }
    catch(const IceUtil::Exception&)
    {
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

Ice::ConnectionI::~ConnectionI()
{
#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    assert(!_startCallback);
#endif
    assert(_state == StateClosed);
    assert(!_transceiver);
#ifndef ICEE_PURE_CLIENT
    assert(_dispatchCount == 0);
#endif
    assert(_requests.empty());
#ifdef ICEE_HAS_AMI
    assert(_asyncRequests.empty());
#endif
}

void
Ice::ConnectionI::setState(State state, const LocalException& ex)
{
    //
    // If setState() is called with an exception, then only closed and
    // closing states are permissible.
    //
    assert(state == StateClosing || state == StateClosed);

    if(_state == state) // Don't switch twice.
    {
        return;
    }

    if(!_exception.get())
    {
        //
        // If we are in closed state, an exception must be set.
        //
        assert(_state != StateClosed);

        _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));

        if(_warn)
        {
            //
            // We don't warn if we are not validated.
            //
            if(_state > StateNotValidated)
            {
                //
                // Don't warn about certain expected exceptions.
                //
                if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                     dynamic_cast<const ForcedCloseConnectionException*>(_exception.get()) ||
                     dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
#ifndef ICEE_PURE_CLIENT
                     dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
#endif
                     (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state == StateClosing)))
                {
                    Warning out(_logger);
                    out << "connection exception:\n" << _exception.get()->toString() << "\n" << _desc;
                }
            }
        }
    }

    //
    // We must set the new state before we notify requests of any
    // exceptions. Otherwise new requests may retry on a connection
    // that is not yet marked as closed or closing.
    //
    setState(state);
}

void
Ice::ConnectionI::setState(State state)
{
    //
    // Skip graceful shutdown if we are destroyed before validation.
    //
    if(_state <= StateNotValidated && state == StateClosing)
    {
        state = StateClosed;
    }

    if(_state == state) // Don't switch twice.
    {
        return;
    }

    switch(state)
    {
    case StateNotInitialized:
    {
        assert(false);
        break;
    }

    case StateNotValidated:
    {
        if(_state != StateNotInitialized)
        {
            assert(_state == StateClosed);
            return;
        }
        break;
    }

    case StateActive:
    {
        //
        // Can only switch from holding or not validated to
        // active.
        //
        if(_state != StateHolding && _state != StateNotValidated)
        {
            return;
        }
        _threadPool->_register(this);
        break;
    }

    case StateHolding:
    {
        //
        // Can only switch from active or not validated to
        // holding.
        //
        if(_state != StateActive && _state != StateNotValidated)
        {
            return;
        }
        _threadPool->unregister(this);
        break;
    }

    case StateClosing:
    {
        //
        // Can't change back from closed.
        //
        if(_state == StateClosed)
        {
            return;
        }
        _threadPool->_register(this); // We need to continue to read in closing state.
        break;
    }

    case StateClosed:
    {
        if(_sendInProgress)
        {
            //
            // Unregister with both the pool and the selector thread. We unregister with
            // the pool to ensure that it stops reading on the socket (otherwise, if the
            // socket is closed the thread pool would spin always reading 0 from the FD).
            // The selector thread will register again the FD with the pool once it's
            // done.
            //
            _selectorThread->finish(this);
            _threadPool->unregister(this);
        }
        else
        {
            _threadPool->finish(this);
        }
        break;
    }
    }

    _state = state;
    _stateTime = IceUtil::Time::now(IceUtil::Time::Monotonic);

    notifyAll();

#ifndef ICEE_PURE_CLIENT
    if(_state == StateClosing && _dispatchCount == 0)
#else
    if(_state == StateClosing)
#endif
    {
        try
        {
            initiateShutdown();
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
        }
    }
}

void
Ice::ConnectionI::initiateShutdown()
{
    assert(_state == StateClosing);
#ifndef ICEE_PURE_CLIENT
    assert(_dispatchCount == 0);
#endif

    //
    // Before we shut down, we send a close connection message.
    //
    BasicStream os(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                   , _instance->initializationData().stringConverter,
                   _instance->initializationData().wstringConverter
#endif
        );
    os.write(magic[0]);
    os.write(magic[1]);
    os.write(magic[2]);
    os.write(magic[3]);
    os.write(protocolMajor);
    os.write(protocolMinor);
    os.write(encodingMajor);
    os.write(encodingMinor);
    os.write(closeConnectionMsg);
    os.write((Byte)0); // Compression status: compression not supported.
    os.write(headerSize); // Message size.

    OutgoingMessage message(&os);
    sendMessage(message);

    //
    // The CloseConnection message should be sufficient. Closing the write
    // end of the socket is probably an artifact of how things were done
    // in IIOP. In fact, shutting down the write end of the socket causes
    // problems on Windows by preventing the peer from using the socket.
    // For example, the peer is no longer able to continue writing a large
    // message after the socket is shutdown.
    //
    //_transceiver->shutdownWrite();
}

SocketStatus
Ice::ConnectionI::initialize()
{
    SocketStatus status = _transceiver->initialize();
    if(status != Finished)
    {
        return status;
    }

    //
    // Update the connection description once the transceiver is initialized.
    //
    const_cast<string&>(_desc) = _transceiver->toString();
    setState(StateNotValidated);
    return Finished;
}

SocketStatus
Ice::ConnectionI::validate()
{
#ifndef ICEE_PURE_CLIENT
    if(_adapter) // The server side has the active role for connection validation.
    {
        BasicStream& os = _stream;
        if(os.b.empty())
        {
            os.write(magic[0]);
            os.write(magic[1]);
            os.write(magic[2]);
            os.write(magic[3]);
            os.write(protocolMajor);
            os.write(protocolMinor);
            os.write(encodingMajor);
            os.write(encodingMinor);
            os.write(validateConnectionMsg);
            os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
            os.write(headerSize); // Message size.
            os.i = os.b.begin();
            traceSend(os, _logger, _traceLevels);
        }

        if(!_transceiver->write(os))
        {
            return NeedWrite;
        }
    }
    else // The client side has the passive role for connection validation.
#endif
    {
        BasicStream& is = _stream;
        if(is.b.empty())
        {
            is.b.resize(headerSize);
            is.i = is.b.begin();
        }

        if(!_transceiver->read(is))
        {
            return NeedRead;
        }

        assert(is.i == is.b.end());
        is.i = is.b.begin();
        Byte m[4];
        is.read(m[0]);
        is.read(m[1]);
        is.read(m[2]);
        is.read(m[3]);
        if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
        {
            throwBadMagicException(__FILE__, __LINE__, Ice::ByteSeq(&m[0], &m[0] + sizeof(magic)));
        }
        Byte pMajor;
        Byte pMinor;
        is.read(pMajor);
        is.read(pMinor);
        if(pMajor != protocolMajor)
        {
            throwUnsupportedProtocolException(__FILE__, __LINE__, pMajor, pMinor, protocolMajor, protocolMinor);
        }
        Byte eMajor;
        Byte eMinor;
        is.read(eMajor);
        is.read(eMinor);
        if(eMajor != encodingMajor)
        {
            throwUnsupportedEncodingException(__FILE__, __LINE__, eMajor, eMinor, encodingMajor, encodingMinor);
        }
        Byte messageType;
        is.read(messageType);
        if(messageType != validateConnectionMsg)
        {
            throwConnectionNotValidatedException(__FILE__, __LINE__);
        }
        Byte compress;
        is.read(compress); // Ignore compression status for validate connection.
        Int size;
        is.read(size);
        if(size != headerSize)
        {
            throwIllegalMessageSizeException(__FILE__, __LINE__);
        }
        traceRecv(is, _logger, _traceLevels);
    }

    _stream.resize(0);
    _stream.i = _stream.b.begin();

    //
    // We start out in holding state.
    //
    setState(StateHolding);
    return Finished;
}

bool
Ice::ConnectionI::send()
{
    assert(_transceiver);
    assert(!_sendStreams.empty());

#ifdef ICEE_HAS_AMI
    bool flushSentCallbacks = _sentCallbacks.empty();
#endif
    try
    {
        while(!_sendStreams.empty())
        {
            OutgoingMessage* message = &_sendStreams.front();

            //
            // Prepare the message stream for writing if necessary.
            //
            if(!message->stream->i)
            {
                message->stream->i = message->stream->b.begin();
                //
                // Message not compressed. Do not compress response, if any.
                //
                message->stream->b[9] = 0;

                //
                // Fill in the message size.
                //
                Int sz = static_cast<Int>(message->stream->b.size());
                const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
                reverse_copy(p, p + sizeof(Int), message->stream->b.begin() + 10);
#else
                copy(p, p + sizeof(Int), message->stream->b.begin() + 10);
#endif
                message->stream->i = message->stream->b.begin();

#ifdef ICEE_HAS_AMI
                if(message->outAsync)
                {
                    trace("sending asynchronous request", *message->stream, _logger, _traceLevels);
                }
                else
#endif
                {

                    traceSend(*message->stream, _logger, _traceLevels);
                }
            }

            //
            // Send the first message.
            //
            assert(message->stream->i);
            if(!_transceiver->write(*message->stream))
            {
#ifdef ICEE_HAS_AMI
                if(flushSentCallbacks && !_sentCallbacks.empty())
                {
                    _threadPool->execute(_flushSentCallbacks);
                }
#endif
                return false;
            }

            //
            // Notify the message that it was sent.
            //
            message->sent(this, true);
#ifdef ICEE_HAS_AMI
            if(dynamic_cast<Ice::AMISentCallback*>(message->outAsync.get()))
            {
                _sentCallbacks.push_back(message->outAsync);
            }
#endif
            _sendStreams.pop_front();
        }
    }
    catch(const Ice::LocalException&)
    {
#ifdef ICEE_HAS_AMI
        if(flushSentCallbacks && !_sentCallbacks.empty())
        {
            _threadPool->execute(_flushSentCallbacks);
        }
#endif
        throw;
    }

#ifdef ICEE_HAS_AMI
    if(flushSentCallbacks && !_sentCallbacks.empty())
    {
        _threadPool->execute(_flushSentCallbacks);
    }
#endif
    return true;
}

#ifdef ICEE_HAS_AMI
void
Ice::ConnectionI::flushSentCallbacks()
{
    vector<OutgoingAsyncMessageCallbackPtr> callbacks;
    {
        Lock sync(*this);
        assert(!_sentCallbacks.empty());
        _sentCallbacks.swap(callbacks);
    }
    for(vector<OutgoingAsyncMessageCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        (*p)->__sentCallback(_instance);
    }
}
#endif

bool
Ice::ConnectionI::sendMessage(OutgoingMessage& message)
{
    assert(_state != StateClosed);

    message.stream->i = 0; // Reset the message stream iterator before starting sending the message.

    if(_sendInProgress)
    {
        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0);
        return false;
    }

    assert(!_sendInProgress);

    //
    // Attempt to send the message without blocking. If the send blocks, we register
    // the connection with the selector thread.
    //

    message.stream->i = message.stream->b.begin();

    //
    // Message not compressed. Do not compress response, if any.
    //
    message.stream->b[9] = 0;

    //
    // Fill in the message size.
    //
    Int sz = static_cast<Int>(message.stream->b.size());
    const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), message.stream->b.begin() + 10);
#else
    copy(p, p + sizeof(Int), message.stream->b.begin() + 10);
#endif
    message.stream->i = message.stream->b.begin();

#ifdef ICEE_HAS_AMI
    if(message.outAsync)
    {
        trace("sending asynchronous request", *message.stream, _logger, _traceLevels);
    }
    else
#endif
    {
        traceSend(*message.stream, _logger, _traceLevels);
    }

    //
    // Send the message without blocking.
    //
    if(_transceiver->write(*message.stream))
    {
        message.sent(this, false);
        return true;
    }

    _sendStreams.push_back(message);
    _sendStreams.back().adopt(0); // Adopt the stream.

    _sendInProgress = true;
    _selectorThread->_register(_transceiver->fd(), this, NeedWrite, _endpoint->timeout());
    return false;
}

void
#ifndef ICEE_PURE_CLIENT
#ifdef ICEE_HAS_AMI
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& invokeNum, Int& requestId, ServantManagerPtr& servantManager,
                              ObjectAdapterPtr& adapter, OutgoingAsyncPtr& outAsync)
#else
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& invokeNum, Int& requestId, ServantManagerPtr& servantManager,
                              ObjectAdapterPtr& adapter)
#endif
#else
#ifdef ICEE_HAS_AMI
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& requestId, OutgoingAsyncPtr& outAsync)
#else
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& requestId)
#endif
#endif
{
    assert(_state > StateNotValidated && _state < StateClosed);

    try
    {
        //
        // We don't need to check magic and version here. This has
        // already been done by the ThreadPool, which provides us
        // with the stream.
        //
        assert(stream.i == stream.b.end());
        stream.i = stream.b.begin() + 8;
        Byte messageType;
        stream.read(messageType);
        Byte compress;
        stream.read(compress);
        if(compress == 2)
        {
            throw FeatureNotSupportedException(__FILE__, __LINE__, "compression");
        }
        stream.i = stream.b.begin() + headerSize;

        switch(messageType)
        {
            case closeConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                setState(StateClosed, CloseConnectionException(__FILE__, __LINE__));
                break;
            }

#ifndef ICEE_PURE_CLIENT
            case requestMsg:
            {
                if(_state == StateClosing)
                {
                    trace("received request during closing\n(ignored by server, client will retry)", stream, _logger,
                          _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);
                    stream.read(requestId);
                    invokeNum = 1;
                    servantManager = _servantManager;
                    adapter = _adapter;
                    ++_dispatchCount;
                }
                break;
            }

            case requestBatchMsg:
            {
                if(_state == StateClosing)
                {
                    trace("received batch request during closing\n(ignored by server, client will retry)", stream,
                          _logger, _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);
                    stream.read(invokeNum);
                    if(invokeNum < 0)
                    {
                        invokeNum = 0;
                        throwNegativeSizeException(__FILE__, __LINE__);
                    }
                    servantManager = _servantManager;
                    adapter = _adapter;
                    _dispatchCount += invokeNum;
                }
                break;
            }
#endif

            case replyMsg:
            {
                traceRecv(stream, _logger, _traceLevels);

                stream.read(requestId);

#ifdef ICEE_HAS_AMI
                map<Int, Outgoing*>::iterator p = _requests.end();
                map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.end();
                if(_requestsHint != _requests.end())
                {
                    if(_requestsHint->first == requestId)
                    {
                        p = _requestsHint;
                    }
                }

                if(p == _requests.end())
                {
                    if(_asyncRequestsHint != _asyncRequests.end())
                    {
                        if(_asyncRequestsHint->first == requestId)
                        {
                            q = _asyncRequestsHint;
                        }
                    }
                }

                if(p == _requests.end() && q == _asyncRequests.end())
                {
                    p = _requests.find(requestId);
                }

                if(p == _requests.end() && q == _asyncRequests.end())
                {
                    q = _asyncRequests.find(requestId);
                }

                if(p == _requests.end() && q == _asyncRequests.end())
                {
                    throwUnknownRequestIdException(__FILE__, __LINE__);
                }

                if(p != _requests.end())
                {
                    p->second->finished(stream);

                    if(p == _requestsHint)
                    {
                        _requests.erase(p++);
                        _requestsHint = p;
                    }
                    else
                    {
                        _requests.erase(p);
                    }
                }
                else
                {
                    assert(q != _asyncRequests.end());

                    outAsync = q->second;

                    if(q == _asyncRequestsHint)
                    {
                        _asyncRequests.erase(q++);
                        _asyncRequestsHint = q;
                    }
                    else
                    {
                        _asyncRequests.erase(q);
                    }
                }
#else
                map<Int, Outgoing*>::iterator p = _requests.end();
                if(_requestsHint != _requests.end())
                {
                    if(_requestsHint->first == requestId)
                    {
                        p = _requestsHint;
                    }
                }

                if(p == _requests.end())
                {
                    p = _requests.find(requestId);
                }

                if(p == _requests.end())
                {
                    throwUnknownRequestIdException(__FILE__, __LINE__);
                }

                p->second->finished(stream);

                if(p == _requestsHint)
                {
                    _requests.erase(p++);
                    _requestsHint = p;
                }
                else
                {
                    _requests.erase(p);
                }
#endif
                break;
            }

            case validateConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if(_warn)
                {
                    Warning out(_logger);
                    out << "ignoring unexpected validate connection message:\n" << _desc;
                }
                break;
            }

            default:
            {
                trace("received unknown message\n(invalid, closing connection)", stream, _logger, _traceLevels);
                throwUnknownMessageException(__FILE__, __LINE__);
                break;
            }
        }
    }
    catch(const SocketException& ex)
    {
        exception(ex);
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

#ifndef ICEE_PURE_CLIENT
void
Ice::ConnectionI::invokeAll(BasicStream& stream, Int invokeNum, Int requestId, const ServantManagerPtr& servantManager,
                           const ObjectAdapterPtr& adapter)
{
    //
    // Note: In contrast to other private or protected methods, this
    // operation must be called *without* the mutex locked.
    //

    try
    {
        while(invokeNum > 0)
        {
            //
            // Prepare the invocation.
            //
            bool response = requestId != 0;
            Incoming in(_instance.get(), this, adapter);
            BasicStream* is = in.is();
            stream.swap(*is);
            BasicStream* os = in.os();

            //
            // Prepare the response if necessary.
            //
            if(response)
            {
                assert(invokeNum == 1); // No further invocations if a response is expected.
                os->writeBlob(replyHdr, sizeof(replyHdr));

                //
                // Add the request ID.
                //
                os->write(requestId);
            }

            in.invoke(response, requestId);

            //
            // If there are more invocations, we need the stream back.
            //
            if(--invokeNum > 0)
            {
                stream.swap(*is);
            }
        }
    }
    catch(const LocalException& ex)
    {
        invokeException(ex, invokeNum);  // Fatal invocation exception
    }
}
#endif
