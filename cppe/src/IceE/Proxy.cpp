// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Communicator.h>
#include <IceE/Proxy.h>
#include <IceE/ProxyFactory.h>
#include <IceE/Outgoing.h>
#include <IceE/OutgoingAsync.h>
#include <IceE/Connection.h>
#include <IceE/Reference.h>
#include <IceE/ConnectRequestHandler.h>
#include <IceE/Instance.h>
#include <IceE/BasicStream.h>
#include <IceE/LocalException.h>
#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterInfo.h>
#   include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorInfo.h>
#   include <IceE/Locator.h>
#endif
#include <IceE/ConnectionI.h> // To convert from ConnectionIPtr to ConnectionPtr in ice_getConnection().

using namespace std;
using namespace Ice;
using namespace IceInternal;

static const string ice_ping_name = "ice_ping";
static const string ice_ids_name = "ice_ids";
static const string ice_id_name = "ice_id";
static const string ice_isA_name = "ice_isA";

void
Ice::__write(::IceInternal::BasicStream* __os, const ::Ice::Context& v, ::Ice::__U__Context)
{
    __os->writeSize(::Ice::Int(v.size()));
    ::Ice::Context::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
        __os->write(p->first);
        __os->write(p->second);
    }
}

void
Ice::__read(::IceInternal::BasicStream* __is, ::Ice::Context& v, ::Ice::__U__Context)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    while(sz--)
    {
        ::std::pair<const  ::std::string, ::std::string> pair;
        __is->read(const_cast< ::std::string&>(pair.first));
        ::Ice::Context::iterator __i = v.insert(v.end(), pair);
        __is->read(__i->second);
    }
}

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId)
{
    if(b)
    {
        ObjectPrx bb = b->ice_facet(f);
        try
        {
            if(bb->ice_isA(typeId))
            {
                return bb;
            }
#ifndef NDEBUG
            else
            {
                assert(typeId != "::Ice::Object");
            }
#endif
        }
        catch(const FacetNotExistException&)
        {
        }
    }
    return 0;
}

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId, const Context& ctx)
{
    if(b)
    {
        ObjectPrx bb = b->ice_facet(f);
        try
        {
            if(bb->ice_isA(typeId, ctx))
            {
                return bb;
            }
#ifndef NDEBUG
            else
            {
                assert(typeId != "::Ice::Object");
            }
#endif
        }
        catch(const FacetNotExistException&)
        {
        }
    }
    return 0;
}

bool
IceProxy::Ice::Object::operator==(const Object& r) const
{
    return _reference == r._reference;
}

bool
IceProxy::Ice::Object::operator<(const Object& r) const
{
    return _reference < r._reference;
}

Int
IceProxy::Ice::Object::ice_getHash() const
{
    return _reference->hash();
}

CommunicatorPtr
IceProxy::Ice::Object::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
IceProxy::Ice::Object::ice_toString() const
{
    return _reference->toString();
}

bool
IceProxy::Ice::Object::ice_isA(const string& __id, const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        RequestHandlerPtr __handler;
        try
        {
            __checkTwowayOnly(ice_isA_name);
            __handler = __getRequestHandler();
            Outgoing __og(__handler.get(), _reference.get(), ice_isA_name, ::Ice::Nonmutating, __context);
            BasicStream* __os = __og.os();
            try
            {
                __os->write(__id, false);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                __og.abort(__ex);
            }
            bool __ret;
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.is();
                if(!__ok)
                {
                    __is->throwUnknownUserException();
                }
                __is->read(__ret);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return __ret;
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__handler, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__handler, __ex, __cnt);
        }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
        catch(...)
        {
            throw;
        }
#endif
    }
}

void
IceProxy::Ice::Object::ice_ping(const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        RequestHandlerPtr __handler;
        try
        {
            __handler = __getRequestHandler();
            Outgoing __og(__handler.get(), _reference.get(), ice_ping_name, ::Ice::Nonmutating, __context);
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.is();
                if(!__ok)
                {
                    __is->throwUnknownUserException();
                }
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return;
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__handler, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__handler, __ex, __cnt);

        }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
        catch(...)
        {
            throw;
        }
#endif
    }
}

vector<string>
IceProxy::Ice::Object::ice_ids(const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        RequestHandlerPtr __handler;
        try
        {
            __checkTwowayOnly(ice_ids_name);
            __handler = __getRequestHandler();
            Outgoing __og(__handler.get(), _reference.get(), ice_ids_name, ::Ice::Nonmutating, __context);
            vector<string> __ret;
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.is();
                if(!__ok)
                {
                    __is->throwUnknownUserException();
                }
                __is->read(__ret, false);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return __ret;
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__handler, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__handler, __ex, __cnt);
        }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
        catch(...)
        {
            throw;
        }
#endif
    }
}

string
IceProxy::Ice::Object::ice_id(const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        RequestHandlerPtr __handler;
        try
        {
            __checkTwowayOnly(ice_id_name);
            __handler = __getRequestHandler();
            Outgoing __og(__handler.get(), _reference.get(), ice_id_name, ::Ice::Nonmutating, __context);
            string __ret;
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.is();
                if(!__ok)
                {
                    __is->throwUnknownUserException();
                }
                __is->read(__ret, false);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return __ret;
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__handler, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__handler, __ex, __cnt);
        }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
        catch(...)
        {
            throw;
        }
#endif
    }
}

Context
IceProxy::Ice::Object::ice_getContext() const
{
    return *_reference->getContext();
}

ObjectPrx
IceProxy::Ice::Object::ice_context(const Context& newContext) const
{
    ObjectPrx proxy = __newInstance();
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
}

Identity
IceProxy::Ice::Object::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrx
IceProxy::Ice::Object::ice_identity(const Identity& newIdentity) const
{
    if(newIdentity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__);
    }
    if(newIdentity == _reference->getIdentity())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = new Object;
        proxy->setup(_reference->changeIdentity(newIdentity));
        return proxy;
    }
}

const string&
IceProxy::Ice::Object::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrx
IceProxy::Ice::Object::ice_facet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = new Object;
        proxy->setup(_reference->changeFacet(newFacet));
        return proxy;
    }
}

#ifdef ICEE_HAS_ROUTER

RouterPrx
IceProxy::Ice::Object::ice_getRouter() const
{
    RouterInfoPtr ri = _reference->getRouterInfo();
    return ri ? ri->getRouter() : RouterPrx();    
}

ObjectPrx
IceProxy::Ice::Object::ice_router(const RouterPrx& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

#endif

string
IceProxy::Ice::Object::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

ObjectPrx
IceProxy::Ice::Object::ice_adapterId(const string& adapterId) const
{
    ReferencePtr ref = _reference->changeAdapterId(adapterId);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

#ifdef ICEE_HAS_LOCATOR

LocatorPrx
IceProxy::Ice::Object::ice_getLocator() const
{
    LocatorInfoPtr ri = _reference->getLocatorInfo();
    return ri ? ri->getLocator() : LocatorPrx();    
}

ObjectPrx
IceProxy::Ice::Object::ice_locator(const LocatorPrx& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

#endif

bool
IceProxy::Ice::Object::ice_isSecure() const
{
    return _reference->getSecure();
}

ObjectPrx
IceProxy::Ice::Object::ice_secure(bool b) const
{
    if(b == _reference->getSecure())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeSecure(b));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_timeout(int t) const
{
    ReferencePtr ref = _reference->changeTimeout(t);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ConnectionPtr
IceProxy::Ice::Object::ice_getConnection()
{
    int __cnt = 0;
    while(true)
    {
        RequestHandlerPtr __handler;
        try
        {
            return __getRequestHandler()->getConnection(true); // Wait for the connection to be established.
        }
        catch(const LocalException& __ex)
        {
            __handleException(__handler, __ex, __cnt);
        }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
        catch(...)
        {
            throw;
        }
#endif
    }
}

ConnectionPtr
IceProxy::Ice::Object::ice_getCachedConnection() const
{
    return const_cast<IceProxy::Ice::Object*>(this)->__getRequestHandler()->getConnection(false);
}

#ifdef ICEE_HAS_BATCH
void
IceProxy::Ice::Object::ice_flushBatchRequests()
{
    //
    // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
    // requests were queued with the connection, they would be lost without being noticed.
    //
    RequestHandlerPtr __handler;
    int __cnt = -1; // Don't retry.
    try
    {
        __handler = __getRequestHandler();
        BatchOutgoing __og(__handler.get(), _reference->getInstance().get());
        __og.invoke();
    }
    catch(const LocalException& __ex)
    {
        __handleException(__handler, __ex, __cnt);
    }
}

#ifdef ICEE_HAS_AMI
bool
IceProxy::Ice::Object::ice_flushBatchRequests_async(const AMI_Object_ice_flushBatchRequestsPtr& cb)
{
    return cb->__invoke(this);
}
#endif
#endif

::IceInternal::RequestHandlerPtr
IceProxy::Ice::Object::__getRequestHandler()
{
    ::IceUtil::Mutex::Lock sync(*this);

    if(!_handler)
    {
#if !defined(ICEE_HAS_AMI) && !defined(ICEE_HAS_BATCH)
        ConnectionIPtr connection  = _reference->getConnection();
        _handler = connection;
#ifdef ICEE_HAS_ROUTER
        RouterInfoPtr ri = _reference->getRouterInfo();
#ifndef ICEE_PURE_CLIENT
        //
        // If we have a router, set the object adapter for this router
        // (if any) to the new connection, so that callbacks from the
        // router can be received over this new connection.
        //
        if(ri && ri->getAdapter())
        {
            connection->setAdapter(_reference->getRouterInfo()->getAdapter());
        }
#endif

        //
        // If this proxy is for a non-local object, and we are using a router, then
        // add this proxy to the router info object.
        //
        if(ri)
        {
            ri->addProxy(this);
        }
#endif
#else
        ConnectRequestHandlerPtr handler = new ConnectRequestHandler(_reference, this);
        _handler = handler->connect();
#endif
    }
    
    return _handler;
}

#if defined(ICEE_HAS_AMI) || defined(ICEE_HAS_BATCH)
void
IceProxy::Ice::Object::__setRequestHandler(const RequestHandlerPtr& handler, const RequestHandlerPtr& previous)
{
    ::IceUtil::Mutex::Lock sync(*this);
    if(previous.get() == _handler.get())
    {
        _handler = handler;
    }
}
#endif

void
IceProxy::Ice::Object::__copyFrom(const ObjectPrx& from)
{
    ReferencePtr ref;
    RequestHandlerPtr handler;

    {
        ::IceUtil::Mutex::Lock sync(*from.get());

        ref = from->_reference;
        handler = from->_handler;
    }

    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_handler);

    _reference = ref;
    _handler = handler;
}

void
IceProxy::Ice::Object::__handleException(const RequestHandlerPtr& handler, 
                                         const LocalException& ex, 
#ifdef ICEE_HAS_AMI                      
                                         OutgoingAsync* out,
#endif
                                         int& cnt)
{
    //
    // Only _handler needs to be mutex protected here.
    //
    {
        ::IceUtil::Mutex::Lock sync(*this);
        if(handler.get() == _handler.get())
        {
            _handler = 0;
        }
    }

    if(cnt == -1) // Don't retry if the retry count is -1.
    {
        ex.ice_throw();
    }

    try
    {
#ifndef ICEE_HAS_AMI
        _reference->getInstance()->proxyFactory()->checkRetryAfterException(ex, _reference, cnt);
#else
        _reference->getInstance()->proxyFactory()->checkRetryAfterException(ex, _reference, out, cnt);
#endif
    }
    catch(const CommunicatorDestroyedException&)
    {
        //
        // The communicator is already destroyed, so we cannot retry.
        //
        ex.ice_throw();
    }
}

void
IceProxy::Ice::Object::__handleExceptionWrapper(const RequestHandlerPtr& handler, 
                                                const LocalExceptionWrapper& ex
#ifdef ICEE_HAS_AMI                      
                                                , OutgoingAsync* out
#endif
    )
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(handler.get() == _handler.get())
        {
            _handler = 0;
        }
    }

    if(!ex.retry())
    {
        ex.get()->ice_throw();
    }

#ifdef ICEE_HAS_AMI
    if(out)
    {
        out->__send();
    }
#endif
}

void
IceProxy::Ice::Object::__handleExceptionWrapperRelaxed(const RequestHandlerPtr& handler,
                                                       const LocalExceptionWrapper& ex, 
#ifdef ICEE_HAS_AMI                      
                                                       OutgoingAsync* out,
#endif
                                                       int& cnt)
{
    if(!ex.retry())
    {
#ifdef ICEE_HAS_AMI
        __handleException(handler, *ex.get(), out, cnt);
#else
        __handleException(handler, *ex.get(), cnt);
#endif
    }
    else
    {
        {
            IceUtil::Mutex::Lock sync(*this);
            if(handler.get() == _handler.get())
            {
                _handler = 0;
            }
        }

#ifdef ICEE_HAS_AMI
        if(out)
        {
            out->__send();
        }
#endif
    }
}

void
IceProxy::Ice::Object::__checkTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this
    // operation.
    //

    if(!ice_isTwoway())
    {
        TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = name;
        throw ex;
    }
}

IceProxy::Ice::Object*
IceProxy::Ice::Object::__newInstance() const
{
    return new Object;
}

ReferenceMode
IceProxy::Ice::Object::getMode() const
{
    return _reference->getMode();
}

ObjectPrx
IceProxy::Ice::Object::changeMode(ReferenceMode newMode) const
{
    if(_reference->getMode() == newMode)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(newMode));
        return proxy;
    }
}

bool
Ice::proxyIdentityLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() < rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() == rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityAndFacetLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();
        
        if(lhsIdentity < rhsIdentity)
        {
            return true;
        }
        else if(rhsIdentity < lhsIdentity)
        {
            return false;
        }
        
        string lhsFacet = lhs->ice_getFacet();
        string rhsFacet = rhs->ice_getFacet();
        
        if(lhsFacet < rhsFacet)
        {
            return true;
        }
        else if(rhsFacet < lhsFacet)
        {
            return false;
        }
        
        return false;
    }
}

bool
Ice::proxyIdentityAndFacetEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();
        
        if(lhsIdentity == rhsIdentity)
        {
            string lhsFacet = lhs->ice_getFacet();
            string rhsFacet = rhs->ice_getFacet();
            
            if(lhsFacet == rhsFacet)
            {
                return true;
            }
        }
        
        return false;
    }
}
