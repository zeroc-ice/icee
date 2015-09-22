// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCATOR_INFO_H
#define ICEE_LOCATOR_INFO_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/LocatorInfoF.h>
#include <IceE/LocatorF.h>
#include <IceE/ProxyF.h>
#include <IceE/EndpointF.h>

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Logger.h>

namespace IceInternal
{

class LocatorManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorManager();

    void destroy();

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    LocatorInfoPtr get(const Ice::LocatorPrx&);

private:

    std::map<Ice::LocatorPrx, LocatorInfoPtr> _table;
    std::map<Ice::LocatorPrx, LocatorInfoPtr>::iterator _tableHint;

    std::map<Ice::Identity, LocatorTablePtr> _locatorTables;
};

class LocatorTable : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    LocatorTable();

    void clear();
    
    bool getAdapterEndpoints(const std::string&, ::std::vector<EndpointPtr>&) const;
    void addAdapterEndpoints(const std::string&, const ::std::vector<EndpointPtr>&);
    ::std::vector<EndpointPtr> removeAdapterEndpoints(const std::string&);

    bool getProxy(const Ice::Identity&, Ice::ObjectPrx&) const;
    void addProxy(const Ice::Identity&, const Ice::ObjectPrx&);
    Ice::ObjectPrx removeProxy(const Ice::Identity&);
    
private:

    std::map<std::string, std::vector<EndpointPtr> > _adapterEndpointsMap;
    std::map<Ice::Identity, Ice::ObjectPrx > _objectMap;
};

class LocatorInfo : public IceUtil::Shared, public IceUtil::Mutex
{
public:

#ifdef ICEE_HAS_AMI
    class GetEndpointsCallback : virtual public IceUtil::Shared
    {
    public:
        
        virtual void locatorInfoEndpoints(const std::vector<EndpointPtr>&, bool) = 0;
        virtual void locatorInfoException(const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<GetEndpointsCallback> GetEndpointsCallbackPtr;
#endif

    LocatorInfo(const Ice::LocatorPrx&, const LocatorTablePtr&);

    void destroy();

    bool operator==(const LocatorInfo&) const;
    bool operator<(const LocatorInfo&) const;

    Ice::LocatorPrx getLocator() const;
    Ice::LocatorRegistryPrx getLocatorRegistry();

#ifdef ICEE_HAS_AMI
    void getEndpoints(const ReferencePtr&, const GetEndpointsCallbackPtr&);
#else
    std::vector<EndpointPtr> getEndpoints(const ReferencePtr&, bool&);
#endif
    void clearCache(const ReferencePtr&);
    void clearObjectCache(const ReferencePtr&);

    //
    // The following methods need to be public for access by AMI callbacks.
    //
#ifndef ICEE_HAS_AMI
    void getEndpointsException(const ReferencePtr&, const Ice::Exception&);
#else
    void getEndpointsException(const ReferencePtr&, const Ice::Exception&, const GetEndpointsCallbackPtr&);
    void getWellKnownObjectEndpoints(const ReferencePtr&, const Ice::ObjectPrx&, bool, const GetEndpointsCallbackPtr&);
#endif
    void getEndpointsTrace(const ReferencePtr&, const std::vector<EndpointPtr>&, bool);

private:

    void trace(const std::string&, const ReferencePtr&, const std::vector<EndpointPtr>&);

    const Ice::LocatorPrx _locator;
    Ice::LocatorRegistryPrx _locatorRegistry;
    const LocatorTablePtr _table;
};

}

#endif

#endif
