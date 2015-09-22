// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONNECTIONI_H
#define ICEE_CONNECTIONI_H

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/Time.h>
#include <IceE/Connection.h>
#include <IceE/ConnectionIF.h>
#include <IceE/OutgoingConnectionFactoryF.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/IncomingConnectionFactoryF.h>
#endif
#include <IceE/InstanceF.h>
#include <IceE/TransceiverF.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/ServantManagerF.h>
#endif
#include <IceE/EndpointF.h>
#include <IceE/LoggerF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/OutgoingAsyncF.h>
#include <IceE/EventHandler.h>
#include <IceE/RequestHandler.h>
#include <IceE/SocketReadyCallback.h>
#include <IceE/SelectorThreadF.h>

#include <deque>
#include <memory>

namespace IceInternal
{

class Outgoing;
#ifdef ICEE_HAS_BATCH
class BatchOutgoing;
#endif
class OutgoingMessageCallback;
#ifdef ICEE_HAS_AMI
class FlushSentCallbacks;
#endif

}

namespace Ice
{

class LocalException;

class ICE_API ConnectionI : public Ice::Connection,
                            public IceInternal::RequestHandler,
                            public IceInternal::EventHandler,
                            public IceInternal::SocketReadyCallback,
                            public IceUtil::Monitor<IceUtil::Mutex>
{
public:

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    class StartCallback : virtual public IceUtil::Shared
    {
    public:

        virtual void connectionStartCompleted(const ConnectionIPtr&) = 0;
        virtual void connectionStartFailed(const ConnectionIPtr&, const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<StartCallback> StartCallbackPtr;
#endif

    enum DestructionReason
    {
#ifndef ICEE_PURE_CLIENT
        ObjectAdapterDeactivated,
#endif
        CommunicatorDestroyed
    };

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    void start(const StartCallbackPtr&);
#else
    void start();
#endif

    void activate();
#ifndef ICEE_PURE_CLIENT
    void hold();
#endif
    void destroy(DestructionReason);
    void close(bool);

    bool isActiveOrHolding() const;
    bool isFinished() const;

    void throwException() const; // Throws the connection exception if destroyed.

#ifndef ICEE_PURE_CLIENT
    void waitUntilHolding() const;
#endif
    void waitUntilFinished(); // Not const, as this might close the connection upon timeout.

#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

#ifndef ICEE_PURE_CLIENT
    void sendResponse(IceInternal::BasicStream*);
    void sendNoResponse();
#endif

    IceInternal::EndpointPtr endpoint() const;

#ifndef ICEE_PURE_CLIENT
    void setAdapter(const ObjectAdapterPtr&); // From Connection.
    ObjectAdapterPtr getAdapter() const; // From Connection.
    ObjectPrx createProxy(const Identity& ident) const; // From Connection.
#endif

    //
    // Inherited from RequestHandler.
    //
    virtual ConnectionI* sendRequest(IceInternal::Outgoing*, bool);
#ifdef ICEE_HAS_AMI
    virtual bool sendAsyncRequest(const IceInternal::OutgoingAsyncPtr&, bool);
#endif
#ifdef ICEE_HAS_BATCH
    virtual void prepareBatchRequest(IceInternal::BasicStream*);
    virtual void finishBatchRequest(IceInternal::BasicStream*);
    virtual void abortBatchRequest();
    virtual bool flushBatchRequests(IceInternal::BatchOutgoing*);
#ifdef ICEE_HAS_AMI
    virtual bool flushAsyncBatchRequests(const IceInternal::BatchOutgoingAsyncPtr&);
#endif
#endif
    virtual Ice::ConnectionIPtr getConnection(bool);

    //
    // Inherited from EventHandler.
    //
    virtual bool readable() const;
    virtual bool read(IceInternal::BasicStream&);
    virtual void message(IceInternal::BasicStream&, const IceInternal::ThreadPoolPtr&);
    virtual void finished(const IceInternal::ThreadPoolPtr&);
    virtual void exception(const LocalException&);
#ifndef ICEE_PURE_CLIENT
    virtual void invokeException(const LocalException&, int);
#endif
    virtual std::string type() const; // From Connection.
    virtual Ice::Int timeout() const; // From Connection.
    virtual std::string toString() const;  // From Connection and EvantHandler.

    //
    // Inherited from SocketReadyCallback.
    //
    virtual IceInternal::SocketStatus socketReady();
    virtual void socketFinished();
    virtual void socketTimeout();

private:

    ConnectionI(const IceInternal::InstancePtr&, const IceInternal::TransceiverPtr&, const IceInternal::EndpointPtr&
#ifndef ICEE_PURE_CLIENT
               , const ObjectAdapterPtr&
#endif
              );
    virtual ~ConnectionI();

#ifndef ICEE_PURE_CLIENT
    friend class IceInternal::IncomingConnectionFactory;
#endif
    friend class IceInternal::OutgoingConnectionFactory;

    enum State
    {
        StateNotInitialized,
        StateNotValidated,
        StateActive,
        StateHolding,
        StateClosing,
        StateClosed
    };

    void setState(State, const LocalException&);
    void setState(State);

    void initiateShutdown();

    struct OutgoingMessage
    {
        OutgoingMessage(IceInternal::BasicStream* str) :
	    stream(str), out(0), response(false), adopted(false)
	{
	}

        OutgoingMessage(IceInternal::OutgoingMessageCallback* o, IceInternal::BasicStream* str, bool resp) :
	    stream(str), out(o), response(resp), adopted(false)
	{
	}

#ifdef ICEE_HAS_AMI
        OutgoingMessage(const IceInternal::OutgoingAsyncMessageCallbackPtr& o, IceInternal::BasicStream* str,
                        bool resp) :
	    stream(str), out(0), outAsync(o), response(resp), adopted(false)
	{
	}
#endif

        void adopt(IceInternal::BasicStream*);
        void sent(ConnectionI*, bool);
        void finished(const Ice::LocalException&);

        IceInternal::BasicStream* stream;
        IceInternal::OutgoingMessageCallback* out;
#ifdef ICEE_HAS_AMI
        IceInternal::OutgoingAsyncMessageCallbackPtr outAsync;
#endif
        bool response;
        bool adopted;
    };

    IceInternal::SocketStatus initialize();
    IceInternal::SocketStatus validate();
    bool send();
#ifdef ICEE_HAS_AMI
    friend class IceInternal::FlushSentCallbacks;
    void flushSentCallbacks();
#endif
    bool sendMessage(OutgoingMessage&);

#ifndef ICEE_PURE_CLIENT
    void parseMessage(IceInternal::BasicStream&, Int&, Int&, IceInternal::ServantManagerPtr&, ObjectAdapterPtr&
#ifdef ICEE_HAS_AMI
                      , IceInternal::OutgoingAsyncPtr&
#endif
        );
    void invokeAll(IceInternal::BasicStream&, Int, Int, const IceInternal::ServantManagerPtr&, const ObjectAdapterPtr&);
#else
    void parseMessage(IceInternal::BasicStream&, Int&
#ifdef ICEE_HAS_AMI
                      , IceInternal::OutgoingAsyncPtr&
#endif
        );
#endif

    IceInternal::TransceiverPtr _transceiver;
    const std::string _desc;
    const std::string _type;
    const IceInternal::EndpointPtr _endpoint;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterPtr _adapter;
    IceInternal::ServantManagerPtr _servantManager;
#endif

    const LoggerPtr _logger;
    const IceInternal::TraceLevelsPtr _traceLevels;
    const IceInternal::ThreadPoolPtr _threadPool;
    const IceInternal::SelectorThreadPtr _selectorThread;

#if defined(ICEE_HAS_AMI) || !defined(ICEE_PURE_CLIENT)
    StartCallbackPtr _startCallback;
#endif

    const bool _warn;

    Int _nextRequestId;

    std::map<Int, IceInternal::Outgoing*> _requests;
    std::map<Int, IceInternal::Outgoing*>::iterator _requestsHint;

#ifdef ICEE_HAS_AMI
    std::map<Int, IceInternal::OutgoingAsyncPtr> _asyncRequests;
    std::map<Int, IceInternal::OutgoingAsyncPtr>::iterator _asyncRequestsHint;
#endif

    std::auto_ptr<LocalException> _exception;

#ifdef ICEE_HAS_BATCH
    const bool _batchAutoFlush;
    IceInternal::BasicStream _batchStream;
    bool _batchStreamInUse;
    int _batchRequestNum;
    size_t _batchMarker;
#endif

    std::deque<OutgoingMessage> _sendStreams;
    bool _sendInProgress;

#ifdef ICEE_HAS_AMI
    std::vector<IceInternal::OutgoingAsyncMessageCallbackPtr> _sentCallbacks;
    IceInternal::ThreadPoolWorkItemPtr _flushSentCallbacks;
#endif

#ifndef ICEE_PURE_CLIENT
    int _dispatchCount;
#endif

    State _state; // The current state.
    IceUtil::Time _stateTime; // The last time when the state was changed.
};

}

#endif
