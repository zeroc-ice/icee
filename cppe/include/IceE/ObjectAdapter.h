// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_H
#define ICEE_OBJECT_ADAPTER_H

#include <IceE/ObjectAdapterF.h>
#include <IceE/InstanceF.h>
#include <IceE/ObjectAdapterFactoryF.h>
#include <IceE/CommunicatorF.h>
#include <IceE/IncomingConnectionFactoryF.h>
#include <IceE/ServantManagerF.h>
#include <IceE/ProxyF.h>
#include <IceE/ObjectF.h>
#include <IceE/EndpointF.h>

#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterF.h>
#   include <IceE/RouterInfoF.h>
#endif

#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorF.h>
#   include <IceE/LocatorInfoF.h>
#endif

#include <IceE/Exception.h>
#include <IceE/Shared.h>
#include <IceE/RecMutex.h>
#include <IceE/Monitor.h>
#include <IceE/Proxy.h>
#include <list>

namespace Ice
{

typedef ::std::map< ::std::string, ::Ice::ObjectPtr> FacetMap;

class ICE_API ObjectAdapter : public IceUtil::Monitor<IceUtil::RecMutex>, public ::IceUtil::Shared
{
public:

    std::string getName() const;

    CommunicatorPtr getCommunicator() const;

    void activate();
    void hold();
    void waitForHold();
    void deactivate();
    void waitForDeactivate();
    bool isDeactivated() const;
    void destroy();

    ObjectPrx add(const ObjectPtr&, const Identity&);
    ObjectPrx addFacet(const ObjectPtr&, const Identity&, const std::string&);
    ObjectPrx addWithUUID(const ObjectPtr&);
    ObjectPrx addFacetWithUUID(const ObjectPtr&, const std::string&);
    void addDefaultServant(const ObjectPtr&, const std::string&);
    ObjectPtr remove(const Identity&);
    ObjectPtr removeFacet(const Identity&, const std::string&);
    FacetMap removeAllFacets(const Identity&);
    ObjectPtr removeDefaultServant(const std::string&);
    ObjectPtr find(const Identity&) const;
    ObjectPtr findFacet(const Identity&, const std::string&) const;
    FacetMap findAllFacets(const Identity&) const;
    ObjectPtr findByProxy(const ObjectPrx&) const;
    ObjectPtr findDefaultServant(const std::string&) const;

    ObjectPrx createProxy(const Identity&) const;
#ifdef ICEE_HAS_LOCATOR
    ObjectPrx createDirectProxy(const Identity&) const;
    ObjectPrx createIndirectProxy(const Identity&) const;
    void setLocator(const LocatorPrx&);
#endif
    void refreshPublishedEndpoints();
    
#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

    IceInternal::ServantManagerPtr getServantManager() const;

private:

    ObjectAdapter(const IceInternal::InstancePtr&, const CommunicatorPtr&, const IceInternal::ObjectAdapterFactoryPtr&, 
                      const std::string&, const std::string&,
#ifdef ICEE_HAS_ROUTER
                  const RouterPrx&,
#endif
                  bool);
    ~ObjectAdapter();
    friend class IceInternal::ObjectAdapterFactory;
    
    ObjectPrx newProxy(const Identity&, const std::string&) const;
    ObjectPrx newDirectProxy(const Identity&, const std::string&) const;
#ifdef ICEE_HAS_LOCATOR
    ObjectPrx newIndirectProxy(const Identity&, const std::string&, const std::string&) const;
#endif
    void checkForDeactivation() const;
    static void checkIdentity(const Identity&);
    std::vector<IceInternal::EndpointPtr> parseEndpoints(const std::string&, bool) const;
    std::vector<IceInternal::EndpointPtr> parsePublishedEndpoints();
#ifdef ICEE_HAS_LOCATOR
    void updateLocatorRegistry(const IceInternal::LocatorInfoPtr&, const Ice::ObjectPrx&);
#endif

    bool _deactivated;
    IceInternal::InstancePtr _instance;
    CommunicatorPtr _communicator;
    IceInternal::ObjectAdapterFactoryPtr _objectAdapterFactory;
    IceInternal::ServantManagerPtr _servantManager;
    bool _activateOneOffDone;
    const std::string _name;
#ifdef ICEE_HAS_LOCATOR
    const std::string _id;
    const std::string _replicaGroupId;
#endif
    IceInternal::ReferencePtr _reference;
    std::vector<IceInternal::IncomingConnectionFactoryPtr> _incomingConnectionFactories;
#ifdef ICEE_HAS_ROUTER
    std::vector<IceInternal::EndpointPtr> _routerEndpoints;
    IceInternal::RouterInfoPtr _routerInfo;
#endif
    std::vector<IceInternal::EndpointPtr> _publishedEndpoints;
#ifdef ICEE_HAS_LOCATOR
    IceInternal::LocatorInfoPtr _locatorInfo;
#endif
    bool _waitForActivate;
    bool _destroying;
    bool _destroyed;
    bool _noConfig;
#ifdef ICEE_HAS_LOCATOR
    Identity _processId;
#endif
};

}

#endif
