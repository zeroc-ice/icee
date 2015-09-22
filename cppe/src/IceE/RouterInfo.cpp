// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_ROUTER

#include <IceE/RouterInfo.h>
#include <IceE/Router.h>
#include <IceE/Reference.h>
#include <IceE/LocalException.h>
#include <IceE/Connection.h> // For ice_getConnection()->timeout().
#include <IceE/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(RouterManager* p) { return p; }
IceUtil::Shared* IceInternal::upCast(RouterInfo* p) { return p; }

IceInternal::RouterManager::RouterManager() :
    _tableHint(_table.end())
{
}

void
IceInternal::RouterManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<const RouterPrx, RouterInfo>(&RouterInfo::destroy));

    _table.clear();
    _tableHint = _table.end();
}

RouterInfoPtr
IceInternal::RouterManager::get(const RouterPrx& rtr)
{
    if(!rtr)
    {
        return 0;
    }

    RouterPrx router = rtr->ice_router(0); // The router cannot be routed.

    IceUtil::Mutex::Lock sync(*this);

    map<RouterPrx, RouterInfoPtr>::iterator p = _table.end();
    
    if(_tableHint != _table.end())
    {
        if(_tableHint->first == router)
        {
            p = _tableHint;
        }
    }
    
    if(p == _table.end())
    {
        p = _table.find(router);
    }

    if(p == _table.end())
    {
        _tableHint = _table.insert(_tableHint, pair<const RouterPrx, RouterInfoPtr>(router, new RouterInfo(router)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

RouterInfoPtr
IceInternal::RouterManager::erase(const RouterPrx& rtr)
{
    RouterInfoPtr info;
    if(rtr)
    {
        RouterPrx router = rtr->ice_router(0); // The router cannot be routed.
        IceUtil::Mutex::Lock sync(*this);

        map<RouterPrx, RouterInfoPtr>::iterator p = _table.end();
        if(_tableHint != _table.end() && _tableHint->first == router)
        {
            p = _tableHint;
            _tableHint = _table.end();
        }
        
        if(p == _table.end())
        {
            p = _table.find(router);
        }
        
        if(p != _table.end())
        {
            info = p->second;
            _table.erase(p);
        }
    }

    return info;
}

IceInternal::RouterInfo::RouterInfo(const RouterPrx& router) :
    _router(router)
{
    assert(_router);
}

void
IceInternal::RouterInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _clientEndpoints.clear();
    _serverEndpoints.clear();
#ifndef ICEE_PURE_CLIENT
    _adapter = 0;
#endif
    _identities.clear();
}

bool
IceInternal::RouterInfo::operator==(const RouterInfo& rhs) const
{
    return _router == rhs._router;
}

bool
IceInternal::RouterInfo::operator<(const RouterInfo& rhs) const
{
    return _router < rhs._router;
}

RouterPrx
IceInternal::RouterInfo::getRouter() const
{
    //
    // No mutex lock necessary, _router is immutable.
    //
    return _router;
}

vector<EndpointPtr>
IceInternal::RouterInfo::getClientEndpoints()
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_clientEndpoints.empty())
        {
            return _clientEndpoints;
        }
    }
    return setClientEndpoints(_router->getClientProxy());
}

#ifdef ICEE_HAS_AMI
void
IceInternal::RouterInfo::getClientEndpoints(const GetClientEndpointsCallbackPtr& callback)
{
    vector<EndpointPtr> clientEndpoints;
    {
        IceUtil::Mutex::Lock sync(*this);
        clientEndpoints = _clientEndpoints;
    }

    if(!clientEndpoints.empty())
    {
        callback->routerInfoEndpoints(clientEndpoints);
        return;
    }

    class Callback : public AMI_Router_getClientProxy
    {
    public:

        virtual void
        ice_response(const Ice::ObjectPrx& clientProxy)
        {
            _callback->routerInfoEndpoints(_routerInfo->setClientEndpoints(clientProxy));
        }

        virtual void
        ice_exception(const Ice::Exception& ex)
        {
            _callback->routerInfoException(dynamic_cast<const Ice::LocalException&>(ex));
        }

        Callback(const RouterInfoPtr& routerInfo, const GetClientEndpointsCallbackPtr& callback) :
            _routerInfo(routerInfo), _callback(callback)
        {
        }

    private:

        const RouterInfoPtr _routerInfo;
        const GetClientEndpointsCallbackPtr _callback;
    };

    _router->getClientProxy_async(new Callback(this, callback));
}
#endif

vector<EndpointPtr>
IceInternal::RouterInfo::getServerEndpoints()
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_serverEndpoints.empty())
        {
            return _serverEndpoints;
        }
    }
    return setServerEndpoints(_router->getServerProxy());
}

#ifndef ICEE_HAS_AMI
void
IceInternal::RouterInfo::addProxy(const Ice::ObjectPrx& proxy)
{
    assert(proxy);
    {
        IceUtil::Mutex::Lock sync(*this);
        if(_identities.find(proxy->ice_getIdentity()) != _identities.end())
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return;
        }
    }

    Ice::ObjectProxySeq proxies;
    proxies.push_back(proxy);
    addAndEvictProxies(proxy, _router->addProxies(proxies));
}

#else

bool
IceInternal::RouterInfo::addProxy(const Ice::ObjectPrx& proxy, const GetClientEndpointsCallbackPtr& callback)
{
    assert(proxy);
    {
        IceUtil::Mutex::Lock sync(*this);
        if(_identities.find(proxy->ice_getIdentity()) != _identities.end())
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return true;
        }
    }

    Ice::ObjectProxySeq proxies;
    proxies.push_back(proxy);

    class Callback : public AMI_Router_addProxies
    {
    public:
        
        virtual void
        ice_response(const Ice::ObjectProxySeq& evictedProxies)
        {
            _routerInfo->addAndEvictProxies(_proxy, evictedProxies);
            _callback->routerInfoAddedProxy();
        }

        virtual void
        ice_exception(const Ice::Exception& ex)
        {
            _callback->routerInfoException(dynamic_cast<const Ice::LocalException&>(ex));
        }

        Callback(const RouterInfoPtr& routerInfo, 
                 const Ice::ObjectPrx& proxy, 
                 const GetClientEndpointsCallbackPtr& callback) :
            _routerInfo(routerInfo), _proxy(proxy), _callback(callback)
        {
        }
        
    private:
        
        const RouterInfoPtr _routerInfo;
        const Ice::ObjectPrx _proxy;
        const GetClientEndpointsCallbackPtr _callback;
    };
    _router->addProxies_async(new Callback(this, proxy, callback), proxies);
    return false;
}
#endif

#ifndef ICEE_PURE_CLIENT
void
IceInternal::RouterInfo::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    _adapter = adapter;
}

ObjectAdapterPtr
IceInternal::RouterInfo::getAdapter() const
{
    IceUtil::Mutex::Lock sync(*this);
    return _adapter;
}
#endif

vector<EndpointPtr>
IceInternal::RouterInfo::setClientEndpoints(const Ice::ObjectPrx& proxy)
{
    IceUtil::Mutex::Lock sync(*this);
    if(_clientEndpoints.empty())
    {
        if(!proxy)
        {
            //
            // If getClientProxy() return nil, use router endpoints.
            //
            _clientEndpoints = _router->__reference()->getEndpoints();
        }
        else
        {
            Ice::ObjectPrx clientProxy = proxy->ice_router(0); // The client proxy cannot be routed.
            
            //
            // In order to avoid creating a new connection to the router,
            // we must use the same timeout as the already existing
            // connection.
            //
            clientProxy = clientProxy->ice_timeout(_router->ice_getConnection()->timeout());
            
            _clientEndpoints = clientProxy->__reference()->getEndpoints();
        }
    }
    return _clientEndpoints;
}

vector<EndpointPtr>
IceInternal::RouterInfo::setServerEndpoints(const Ice::ObjectPrx& serverProxy)
{
    IceUtil::Mutex::Lock sync(*this);
    if(_serverEndpoints.empty()) // Lazy initialization.
    {
        ObjectPrx serverProxy = _router->getServerProxy();
        if(!serverProxy)
        {
            throw NoEndpointException(__FILE__, __LINE__);
        }

        serverProxy = serverProxy->ice_router(0); // The server proxy cannot be routed.

        _serverEndpoints = serverProxy->__reference()->getEndpoints();
    }
    return _serverEndpoints;

}

void
IceInternal::RouterInfo::addAndEvictProxies(const Ice::ObjectPrx& proxy, const Ice::ObjectProxySeq& evictedProxies)
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // Check if the proxy hasn't already been evicted by a concurrent addProxies call. 
    // If it's the case, don't add it to our local map.
    //
    multiset<Identity>::iterator p = _evictedIdentities.find(proxy->ice_getIdentity());
    if(p != _evictedIdentities.end())
    {
        _evictedIdentities.erase(p);
    }
    else
    {
        //
        // If we successfully added the proxy to the router,
        // we add it to our local map.
        //
        _identities.insert(proxy->ice_getIdentity());
    }
    
    //
    // We also must remove whatever proxies the router evicted.
    //
    for(Ice::ObjectProxySeq::const_iterator q = evictedProxies.begin(); q != evictedProxies.end(); ++q)
    {
        if(_identities.erase((*q)->ice_getIdentity()) == 0)
        {
            //
            // It's possible for the proxy to not have been
            // added yet in the local map if two threads
            // concurrently call addProxies.
            //
            _evictedIdentities.insert((*q)->ice_getIdentity());
        }
    }
}

#endif
