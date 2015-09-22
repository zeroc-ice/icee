// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REFERENCE_H
#define ICEE_REFERENCE_H

#include <IceE/ReferenceF.h>
#include <IceE/EndpointF.h>
#include <IceE/InstanceF.h>
#include <IceE/CommunicatorF.h>
#ifdef ICEE_HAS_ROUTER
#  include <IceE/RouterInfoF.h>
#  include <IceE/RouterF.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#  include <IceE/LocatorInfoF.h>
#  include <IceE/LocatorF.h>
#endif
#include <IceE/ConnectionIF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Identity.h>

namespace IceInternal
{

class BasicStream;

class ConnectRequestHandler;
typedef IceUtil::Handle<ConnectRequestHandler> ConnectRequestHandlerPtr;

class Reference : public IceUtil::Shared
{
public:

    //
    // The reference mode in Ice-E is defined in ReferenceF.h to allow the proxy
    // to inline methods such as ice_twoway, ice_isTwoway, etc.
    //
//     enum Mode
//     {
//         ModeTwoway,
//         ModeOneway,
//         ModeBatchOneway,
//         ModeDatagram,
//         ModeBatchDatagram,
//         ModeLast = ModeBatchDatagram
//     };

    ReferenceMode getMode() const { return _mode; }
    bool getSecure() const { return _secure; };
    const Ice::Identity& getIdentity() const { return _identity; }
    const std::string& getFacet() const { return _facet; }
    const InstancePtr& getInstance() const { return _instance; }
    const Ice::Context* getContext() const { return &_context; }

    Ice::CommunicatorPtr getCommunicator() const;

    virtual std::vector<EndpointPtr> getEndpoints() const = 0;

    virtual std::string getAdapterId() const = 0;
#ifdef ICEE_HAS_LOCATOR
    virtual LocatorInfoPtr getLocatorInfo() const { return 0; }
#endif
#ifdef ICEE_HAS_ROUTER
    virtual RouterInfoPtr getRouterInfo() const { return 0; }
#endif

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeMode(ReferenceMode) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeFacet(const std::string&) const;

    virtual ReferencePtr changeAdapterId(const std::string&) const = 0;
#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const = 0;
#endif
#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const = 0;
#endif

    virtual ReferencePtr changeTimeout(int) const = 0;

    int hash() const; // Conceptually const.

    //
    // Utility methods.
    //
    virtual bool isIndirect() const = 0;
    virtual bool isWellKnown() const = 0;

    //
    // Marshal the reference.
    //
    virtual void streamWrite(BasicStream*) const;

    //
    // Convert the reference to its string form.
    //
    virtual std::string toString() const;

    //
    // Get a suitable connection for this reference.
    //
#ifndef ICEE_HAS_AMI
    virtual Ice::ConnectionIPtr getConnection() const = 0;
#else
    virtual void getConnection(const ConnectRequestHandlerPtr&) const = 0;
#endif

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
              const std::string&, ReferenceMode, bool);
    Reference(const Reference&);

    void hashInit() const;

    IceUtil::Mutex _hashMutex; // For lazy initialization of hash value.
    mutable Ice::Int _hashValue;
    mutable bool _hashInitialized;

private:

    const InstancePtr _instance;
    const Ice::CommunicatorPtr _communicator;

    ReferenceMode _mode;
    bool _secure;
    Ice::Identity _identity;
    Ice::Context _context;
    std::string _facet;
};

class FixedReference : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
                   const std::string&, ReferenceMode, bool, const Ice::ConnectionIPtr&);

    virtual std::vector<EndpointPtr> getEndpoints() const;

    virtual std::string getAdapterId() const;
    virtual ReferencePtr changeAdapterId(const std::string&) const;
#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
#endif

#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
#endif

    virtual ReferencePtr changeTimeout(int) const;

    virtual bool isIndirect() const;
    virtual bool isWellKnown() const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;

#ifndef ICEE_HAS_AMI
    virtual Ice::ConnectionIPtr getConnection() const;
#else
    virtual void getConnection(const ConnectRequestHandlerPtr&) const;
#endif

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

private:

    FixedReference(const FixedReference&);

    Ice::ConnectionIPtr _fixedConnection;
};

class RoutableReference : public Reference
{
public:

    RoutableReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const Ice::Context&,
                      const std::string&, ReferenceMode, bool, const std::vector<EndpointPtr>&, const std::string&
#ifdef ICEE_HAS_LOCATOR
                      , const LocatorInfoPtr&
#endif
#ifdef ICEE_HAS_ROUTER
                      , const RouterInfoPtr&
#endif
    );

    virtual std::vector<EndpointPtr> getEndpoints() const;
    virtual std::string getAdapterId() const;
#ifdef ICEE_HAS_LOCATOR
    virtual LocatorInfoPtr getLocatorInfo() const;
#endif
#ifdef ICEE_HAS_ROUTER
    virtual RouterInfoPtr getRouterInfo() const;
#endif

    virtual ReferencePtr changeAdapterId(const std::string&) const;
#ifdef ICEE_HAS_LOCATOR
    virtual ReferencePtr changeLocator(const Ice::LocatorPrx&) const;
#endif
#ifdef ICEE_HAS_ROUTER
    virtual ReferencePtr changeRouter(const Ice::RouterPrx&) const;
#endif

    virtual ReferencePtr changeTimeout(int) const;

    int hash() const; // Conceptually const.

    virtual bool isIndirect() const;
    virtual bool isWellKnown() const;

    virtual void streamWrite(BasicStream*) const;
    virtual std::string toString() const;

    virtual bool operator==(const Reference&) const;
    virtual bool operator!=(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const;

#ifndef ICEE_HAS_AMI
    virtual Ice::ConnectionIPtr getConnection() const;
    Ice::ConnectionIPtr createConnection(const std::vector<EndpointPtr>&) const;
#else
    virtual void getConnection(const ConnectRequestHandlerPtr&) const;
    virtual void getConnectionNoRouterInfo(const ConnectRequestHandlerPtr&) const;
    void createConnection(const std::vector<EndpointPtr>&, const ConnectRequestHandlerPtr&) const;
#endif

    void applyOverrides(std::vector<EndpointPtr>&) const;

protected:

    RoutableReference(const RoutableReference&);

    std::vector<EndpointPtr> filterEndpoints(const std::vector<EndpointPtr>&) const;

private:

    std::vector<EndpointPtr> _endpoints; // Empty if indirect proxy.

    std::string _adapterId; // Empty if direct proxy.

#ifdef ICEE_HAS_LOCATOR
    LocatorInfoPtr _locatorInfo; // Null if no locator is used.
#endif
#ifdef ICEE_HAS_ROUTER
    RouterInfoPtr _routerInfo; // Null if no router is used.
#endif

    bool _overrideTimeout;
    int _timeout; // Only used if _overrideTimeout == true
};

}

#endif
