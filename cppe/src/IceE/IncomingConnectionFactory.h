
// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INCOMING_CONNECTION_FACTORY_H
#define ICEE_INCOMING_CONNECTION_FACTORY_H

#include <IceE/IncomingConnectionFactoryF.h>
#include <IceE/EndpointF.h>
#include <IceE/ConnectionIF.h>
#include <IceE/ObjectAdapterF.h>
#include <IceE/InstanceF.h>
#include <IceE/AcceptorF.h>
#include <IceE/TransceiverF.h>
#include <IceE/EventHandler.h>
#include <IceE/ConnectionI.h>

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/Shared.h>
#include <IceE/Thread.h>
#include <list>

namespace IceInternal
{

class IncomingConnectionFactory : public EventHandler, 
                                  public Ice::ConnectionI::StartCallback,
                                  public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void activate();
    void hold();
    void destroy();

    void waitUntilHolding() const;
    void waitUntilFinished();

    EndpointPtr endpoint() const;
#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

    //
    // Operations from EventHandler
    //
    virtual bool readable() const;
    virtual bool read(BasicStream&);
    virtual void message(BasicStream&, const ThreadPoolPtr&);
    virtual void finished(const ThreadPoolPtr&);
    virtual void exception(const Ice::LocalException&);
    virtual std::string toString() const;

    virtual void connectionStartCompleted(const Ice::ConnectionIPtr&);
    virtual void connectionStartFailed(const Ice::ConnectionIPtr&, const Ice::LocalException&);

private:

    IncomingConnectionFactory(const InstancePtr&, const EndpointPtr&, const ::Ice::ObjectAdapterPtr&);
    virtual ~IncomingConnectionFactory();
    friend class ::Ice::ObjectAdapter;

    enum State
    {
        StateActive,
        StateHolding,
        StateClosed
    };

    void setState(State);

    AcceptorPtr _acceptor;
    const EndpointPtr _endpoint;

    Ice::ObjectAdapterPtr _adapter;

    const bool _warn;

    std::list<Ice::ConnectionIPtr> _connections;

    State _state;
};

}

#endif
