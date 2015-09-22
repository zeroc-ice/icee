// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INSTANCE_H
#define ICEE_INSTANCE_H

#include <IceE/InstanceF.h>
#include <IceE/CommunicatorF.h>
#include <IceE/PropertiesF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/DefaultsAndOverridesF.h>
#include <IceE/RouterInfoF.h>
#include <IceE/LocatorInfoF.h>
#include <IceE/ReferenceFactoryF.h>
#include <IceE/ProxyFactoryF.h>
#include <IceE/ThreadPoolF.h>
#include <IceE/SelectorThreadF.h>
#include <IceE/OutgoingConnectionFactoryF.h>
#include <IceE/EndpointFactoryF.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapterFactoryF.h>
#endif
#include <IceE/ObjectFactoryManagerF.h>
#ifdef ICEE_HAS_AMI
#   include <IceE/RetryQueueF.h>
#endif
#include <IceE/EndpointF.h>
#include <IceE/Shared.h>
#include <IceE/RecMutex.h>
#include <IceE/Initialize.h>
#include <IceE/Identity.h>
#include <IceE/Timer.h>

namespace IceInternal
{

class Instance : public IceUtil::Shared, public IceUtil::RecMutex
{
public:

    bool destroyed() const;
    const Ice::InitializationData& initializationData() const { return _initData; }
    TraceLevelsPtr traceLevels() const;
    DefaultsAndOverridesPtr defaultsAndOverrides() const;
#ifdef ICEE_HAS_ROUTER
    RouterManagerPtr routerManager() const;
#endif
#ifdef ICEE_HAS_LOCATOR
    LocatorManagerPtr locatorManager() const;
#endif
    ReferenceFactoryPtr referenceFactory() const;
    ProxyFactoryPtr proxyFactory() const;
    OutgoingConnectionFactoryPtr outgoingConnectionFactory() const;
    EndpointFactoryPtr endpointFactory() const;
    size_t messageSizeMax() const { return _messageSizeMax; /* Immutable */ }  // Inlined for performance reasons.
    Ice::Int connectionIdleTime() const;
#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

    ObjectFactoryManagerPtr servantFactoryManager() const;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr objectAdapterFactory() const;
#endif

#ifdef ICEE_HAS_AMI
    RetryQueuePtr retryQueue() const;
    EndpointHostResolverPtr endpointHostResolver();
#endif

    ThreadPoolPtr clientThreadPool();
#ifndef ICEE_PURE_CLIENT
    ThreadPoolPtr serverThreadPool();
#endif
    SelectorThreadPtr selectorThread();
    IceUtil::TimerPtr timer();

    Ice::Identity stringToIdentity(const std::string&) const;
    std::string identityToString(const Ice::Identity&) const;

#ifdef ICEE_HAS_LOCATOR
    void setDefaultLocator(const Ice::LocatorPrx&);
#endif
#ifdef ICEE_HAS_ROUTER
    void setDefaultRouter(const Ice::RouterPrx&);
#endif

private:

    Instance(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
    virtual ~Instance();

    void finishSetup(int&, char*[]);
    void destroy();
    friend class Ice::Communicator;

    enum State
    {
        StateActive,
        StateDestroyInProgress,
        StateDestroyed
    };
    State _state;
    Ice::InitializationData _initData; // Immutable, not reset by destroy().
    const TraceLevelsPtr _traceLevels; // Immutable, not reset by destroy().
    const DefaultsAndOverridesPtr _defaultsAndOverrides; // Immutable, not reset by destroy().
    const size_t _messageSizeMax; // Immutable, not reset by destroy().
#ifdef ICEE_HAS_ROUTER
    RouterManagerPtr _routerManager;
#endif
#ifdef ICEE_HAS_LOCATOR
    LocatorManagerPtr _locatorManager;
#endif
    ReferenceFactoryPtr _referenceFactory;
    ProxyFactoryPtr _proxyFactory;
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    EndpointFactoryPtr _endpointFactory;
    ObjectFactoryManagerPtr _servantFactoryManager;

#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr _objectAdapterFactory;
#endif

#ifdef ICEE_HAS_AMI
    RetryQueuePtr _retryQueue;
    EndpointHostResolverPtr _endpointHostResolver;
#endif

    ThreadPoolPtr _clientThreadPool;
#ifndef ICEE_PURE_CLIENT
    ThreadPoolPtr _serverThreadPool;
#endif
    SelectorThreadPtr _selectorThread;
    IceUtil::TimerPtr _timer;
};

#ifdef ICEE_HAS_WSTRING
class UTF8BufferI : public Ice::UTF8Buffer
{
public:

   UTF8BufferI();
   ~UTF8BufferI();

   Ice::Byte* getMoreBytes(size_t howMany, Ice::Byte* firstUnused);
   Ice::Byte* getBuffer();
   void reset();

private:

    Ice::Byte* _buffer;
    size_t _offset;
};
#endif

}

#endif
