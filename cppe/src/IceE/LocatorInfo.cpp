// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR

#include <IceE/LocatorInfo.h>
#include <IceE/Locator.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Endpoint.h>
#include <IceE/Reference.h>
#include <IceE/Functional.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(LocatorManager* p) { return p; }
IceUtil::Shared* IceInternal::upCast(LocatorInfo* p) { return p; }
IceUtil::Shared* IceInternal::upCast(LocatorTable* p) { return p; }

#ifdef ICEE_HAS_AMI

namespace
{

class WellKnownObjectEndpoints : public LocatorInfo::GetEndpointsCallback
{
public:
    
    virtual void
    locatorInfoEndpoints(const vector<EndpointPtr>& endpoints, bool endpointsCached)
    {
        if(!_objectCached && !endpoints.empty())
        {
            _table->addProxy(_reference->getIdentity(), _object);
        }
        
        if(_reference->getInstance()->traceLevels()->location >= 1)
        {
                _locatorInfo->getEndpointsTrace(_reference, endpoints, _objectCached || endpointsCached);
        }
        
        _callback->locatorInfoEndpoints(endpoints, _objectCached || endpointsCached);
    }
        
    virtual void
    locatorInfoException(const Ice::LocalException& ex)
    {
        _callback->locatorInfoException(ex);
    }


    WellKnownObjectEndpoints(const LocatorInfoPtr& locatorInfo,
                             const LocatorTablePtr& table, 
                             const ReferencePtr& reference,
                             const Ice::ObjectPrx& object,
                             bool objectCached, 
                             const LocatorInfo::GetEndpointsCallbackPtr& callback) :
        _locatorInfo(locatorInfo), 
        _table(table), 
        _reference(reference), 
        _object(object),
        _objectCached(objectCached),
        _callback(callback)
    {
    }

private:
    
    const LocatorInfoPtr _locatorInfo;
    const LocatorTablePtr _table;
    const ReferencePtr _reference;
    const Ice::ObjectPrx _object;
    const bool _objectCached;
    const LocatorInfo::GetEndpointsCallbackPtr _callback;
};

}

#endif

IceInternal::LocatorManager::LocatorManager() :
    _tableHint(_table.end())
{
}

void
IceInternal::LocatorManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<const LocatorPrx, LocatorInfo>(&LocatorInfo::destroy));

    _table.clear();
    _tableHint = _table.end();

    _locatorTables.clear();
}

LocatorInfoPtr
IceInternal::LocatorManager::get(const LocatorPrx& loc)
{
    if(!loc)
    {
        return 0;
    }

    LocatorPrx locator = loc->ice_locator(0); // The locator can't be located.

    //
    // TODO: reap unused locator info objects?
    //

    IceUtil::Mutex::Lock sync(*this);

    map<LocatorPrx, LocatorInfoPtr>::iterator p = _table.end();
    
    if(_tableHint != _table.end())
    {
        if(_tableHint->first == locator)
        {
            p = _tableHint;
        }
    }
    
    if(p == _table.end())
    {
        p = _table.find(locator);
    }

    if(p == _table.end())
    {
        //
        // Rely on locator identity for the adapter table. We want to
        // have only one table per locator (not one per locator
        // proxy).
        //
        map<Identity, LocatorTablePtr>::iterator t = _locatorTables.find(locator->ice_getIdentity());
        if(t == _locatorTables.end())
        {
            t = _locatorTables.insert(_locatorTables.begin(),
                                      pair<const Identity, LocatorTablePtr>(locator->ice_getIdentity(),
                                      new LocatorTable()));
        }

        _tableHint = _table.insert(_tableHint, 
                                pair<const LocatorPrx, LocatorInfoPtr>(locator, new LocatorInfo(locator, t->second)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

IceInternal::LocatorTable::LocatorTable()
{
}

void
IceInternal::LocatorTable::clear()
{
    IceUtil::Mutex::Lock sync(*this);

    _adapterEndpointsMap.clear();
    _objectMap.clear();
}

bool
IceInternal::LocatorTable::getAdapterEndpoints(const string& adapter, vector<EndpointPtr>& endpoints) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, vector<EndpointPtr> >::const_iterator p = _adapterEndpointsMap.find(adapter);
    
    if(p != _adapterEndpointsMap.end())
    {
        endpoints = p->second;
        return true;
    }
    else
    {
        return false;
    }
}

void
IceInternal::LocatorTable::addAdapterEndpoints(const string& adapter, const vector<EndpointPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);
    
    _adapterEndpointsMap.insert(make_pair(adapter, endpoints));
}

vector<EndpointPtr>
IceInternal::LocatorTable::removeAdapterEndpoints(const string& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<string, vector<EndpointPtr> >::iterator p = _adapterEndpointsMap.find(adapter);
    if(p == _adapterEndpointsMap.end())
    {
        return vector<EndpointPtr>();
    }

    vector<EndpointPtr> endpoints = p->second;

    _adapterEndpointsMap.erase(p);
    
    return endpoints;
}

bool 
IceInternal::LocatorTable::getProxy(const Identity& id, ObjectPrx& proxy) const
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<Identity, ObjectPrx>::const_iterator p = _objectMap.find(id);
    
    if(p != _objectMap.end())
    {
        proxy = p->second;
        return true;
    }
    else
    {
        return false;
    }
}

void 
IceInternal::LocatorTable::addProxy(const Identity& id, const ObjectPrx& proxy)
{
    IceUtil::Mutex::Lock sync(*this);
    _objectMap.insert(make_pair(id, proxy));
}

ObjectPrx 
IceInternal::LocatorTable::removeProxy(const Identity& id)
{
    IceUtil::Mutex::Lock sync(*this);
    
    map<Identity, ObjectPrx>::iterator p = _objectMap.find(id);
    if(p == _objectMap.end())
    {
        return 0;
    }

    ObjectPrx proxy = p->second;
    _objectMap.erase(p);
    return proxy;
}

IceInternal::LocatorInfo::LocatorInfo(const LocatorPrx& locator, const LocatorTablePtr& table) :
    _locator(locator),
    _table(table)
{
    assert(_locator);
    assert(_table);
}

void
IceInternal::LocatorInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _locatorRegistry = 0;
    _table->clear();
}

bool
IceInternal::LocatorInfo::operator==(const LocatorInfo& rhs) const
{
    return _locator == rhs._locator;
}

bool
IceInternal::LocatorInfo::operator<(const LocatorInfo& rhs) const
{
    return _locator < rhs._locator;
}

LocatorPrx
IceInternal::LocatorInfo::getLocator() const
{
    //
    // No mutex lock necessary, _locator is immutable.
    //
    return _locator;
}

LocatorRegistryPrx
IceInternal::LocatorInfo::getLocatorRegistry()
{
    IceUtil::Mutex::Lock sync(*this);
    
    if(!_locatorRegistry) // Lazy initialization.
    {
        _locatorRegistry = _locator->getRegistry();

        //
        // The locator registry can't be located.
        //
        _locatorRegistry = LocatorRegistryPrx::uncheckedCast(_locatorRegistry->ice_locator(0));
    }
    
    return _locatorRegistry;
}

#ifndef ICEE_HAS_AMI
vector<EndpointPtr>
IceInternal::LocatorInfo::getEndpoints(const ReferencePtr& ref, bool& cached)
{
    assert(ref->isIndirect());
    vector<EndpointPtr> endpoints;
    ObjectPrx object;
    try
    {
        if(!ref->isWellKnown())
        {
            if(!_table->getAdapterEndpoints(ref->getAdapterId(), endpoints))
            {
                cached = false;
            
                if(ref->getInstance()->traceLevels()->location >= 1)
                {
                    Trace out(ref->getInstance()->initializationData().logger,
                              ref->getInstance()->traceLevels()->locationCat);
                    out << "searching for adapter by id" << "\n";
                    out << "adapter = " << ref->getAdapterId();
                }

                object = _locator->findAdapterById(ref->getAdapterId());
                if(object)
                {
                    endpoints = object->__reference()->getEndpoints();
                    _table->addAdapterEndpoints(ref->getAdapterId(), endpoints);
                }
            }
        }
        else
        {
            bool objectCached = true;
            if(!_table->getProxy(ref->getIdentity(), object))
            {
                if(ref->getInstance()->traceLevels()->location >= 1)
                {
                    Trace out(ref->getInstance()->initializationData().logger,
                              ref->getInstance()->traceLevels()->locationCat);
                    out << "searching for object by id" << "\n";
                    out << "object = " << ref->getInstance()->identityToString(ref->getIdentity());
                }

                objectCached = false;
                object = _locator->findObjectById(ref->getIdentity());
            }

            bool endpointsCached = true;
            if(object)
            {
                ReferencePtr r = object->__reference();
                if(!r->isIndirect())
                {
                    endpointsCached = false;
                    endpoints = r->getEndpoints();
                }
                else if(!r->isWellKnown())
                {
                    endpoints = getEndpoints(r, endpointsCached);
                }
            }

            if(!objectCached && !endpoints.empty())
            {
                _table->addProxy(ref->getIdentity(), object);
            }

            cached = objectCached || endpointsCached;
        }
    }
    catch(const Ice::Exception& ex)
    {
        getEndpointsException(ref, ex);
    }

    if(ref->getInstance()->traceLevels()->location >= 1)
    {
        getEndpointsTrace(ref, endpoints, cached);
    }

    return endpoints;
}

#else

void
IceInternal::LocatorInfo::getEndpoints(const ReferencePtr& ref, const GetEndpointsCallbackPtr& callback)
{
    assert(ref->isIndirect());

    string adapterId = ref->getAdapterId();
    Ice::Identity identity = ref->getIdentity();
    InstancePtr instance = ref->getInstance();
    if(!adapterId.empty())
    {
        vector<EndpointPtr> endpoints;
        if(!_table->getAdapterEndpoints(adapterId, endpoints))
        {
            if(instance->traceLevels()->location >= 1)
            {
                Trace out(instance->initializationData().logger, instance->traceLevels()->locationCat);
                out << "searching for adapter by id" << "\nadapter = " << adapterId;
            }

            class Callback : public AMI_Locator_findAdapterById
            {
            public:

                virtual void
                ice_response(const Ice::ObjectPrx& object)
                {
                    vector<EndpointPtr> endpoints;
                    if(object)
                    {
                        endpoints = object->__reference()->getEndpoints();
                        if(!endpoints.empty())
                        {
                            _table->addAdapterEndpoints(_reference->getAdapterId(), endpoints);
                        }
                    }
                    
                    if(_reference->getInstance()->traceLevels()->location >= 1)
                    {
                        _locatorInfo->getEndpointsTrace(_reference, endpoints, false);
                    }
                    
                    _callback->locatorInfoEndpoints(endpoints, false);
                }

                virtual void
                ice_exception(const Ice::Exception& ex)
                {
                    _locatorInfo->getEndpointsException(_reference, ex, _callback);
                }

                Callback(const LocatorInfoPtr& locatorInfo, const LocatorTablePtr& table,
                         const ReferencePtr& reference, const GetEndpointsCallbackPtr& callback) :
                    _locatorInfo(locatorInfo), _table(table), _reference(reference), _callback(callback)
                {
                }
                
            private:

                const LocatorInfoPtr _locatorInfo;
                const LocatorTablePtr _table;
                const ReferencePtr _reference;
                const GetEndpointsCallbackPtr _callback;
            };

            //
            // Search the adapter in the location service if we didn't
            // find it in the cache.
            //
            _locator->findAdapterById_async(new Callback(this, _table, ref, callback), adapterId);
            return;
        }
        else
        {
            if(instance->traceLevels()->location >= 1)
            {
                getEndpointsTrace(ref, endpoints, true);
            }
            callback->locatorInfoEndpoints(endpoints, true);
            return;
        }
    }
    else
    {
        Ice::ObjectPrx object;
        if(!_table->getProxy(identity, object))
        {
            if(instance->traceLevels()->location >= 1)
            {
                Trace out(instance->initializationData().logger, instance->traceLevels()->locationCat);
                out << "searching for object by id" << "\nobject = " << instance->identityToString(ref->getIdentity());
            }

            class Callback : public Ice::AMI_Locator_findObjectById
            {
            public:
                
                virtual void
                ice_response(const Ice::ObjectPrx& object)
                {
                    _locatorInfo->getWellKnownObjectEndpoints(_reference, object, false, _callback);
                }
                
                virtual void
                ice_exception(const Ice::Exception& ex)
                {
                    _locatorInfo->getEndpointsException(_reference, ex, _callback);
                }

                Callback(const LocatorInfoPtr& locatorInfo, const ReferencePtr& reference, 
                         const GetEndpointsCallbackPtr& callback) :
                    _locatorInfo(locatorInfo), _reference(reference), _callback(callback)
                {
                }

            private:
                
                const LocatorInfoPtr _locatorInfo;
                const ReferencePtr _reference;
                const GetEndpointsCallbackPtr _callback;
            };

            _locator->findObjectById_async(new Callback(this, ref, callback), identity);
            return;
        }
        else
        {
            getWellKnownObjectEndpoints(ref, object, true, callback);
            return;
        }
    }
}

#endif

void
IceInternal::LocatorInfo::clearObjectCache(const ReferencePtr& ref)
{
    assert(ref->isIndirect());

    if(ref->isWellKnown())
    {
        ObjectPrx object = _table->removeProxy(ref->getIdentity());
        if(object)
        {
            ReferencePtr r = object->__reference();
            if(!r->isIndirect())
            {
                if(ref->getInstance()->traceLevels()->location >= 2)
                {
                    trace("removed endpoints from locator table", ref, r->getEndpoints());
                }
            }
            else if(!r->isWellKnown())
            {
                clearCache(r);
            }
        }
    }
}

void 
IceInternal::LocatorInfo::clearCache(const ReferencePtr& ref)
{
    assert(ref->isIndirect());

    if(!ref->isWellKnown())
    {
        vector<EndpointPtr> endpoints = _table->removeAdapterEndpoints(ref->getAdapterId());

        if(!endpoints.empty() && ref->getInstance()->traceLevels()->location >= 2)
        {
            trace("removed endpoints from locator table", ref, endpoints);
        }
    }
    else
    {
        ObjectPrx object = _table->removeProxy(ref->getIdentity());
        if(object)
        {
            ReferencePtr r = object->__reference();
            if(!r->isIndirect())
            {
                if(ref->getInstance()->traceLevels()->location >= 2)
                {
                    trace("removed endpoints from locator table", ref, r->getEndpoints());
                }
            }
            else if(!r->isWellKnown())
            {
                clearCache(r);
            }
        }
    }
}

void
IceInternal::LocatorInfo::trace(const string& msg, const ReferencePtr& ref, const vector<EndpointPtr>& endpoints)
{
    assert(ref->isIndirect());

    Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
    out << msg << '\n';
    if(!ref->isWellKnown())
    {
        out << "adapter = "  << ref->getAdapterId() << '\n';
    }
    else
    {
        out << "object = "  << ref->getInstance()->identityToString(ref->getIdentity()) << '\n';
    }

    const char* sep = endpoints.size() > 1 ? ":" : "";
    out << "endpoints = ";
    for(unsigned int i = 0; i < endpoints.size(); ++i)
    {
        out << endpoints[i]->toString() << sep;
    }
}

void 
IceInternal::LocatorInfo::getEndpointsException(const ReferencePtr& ref, 
                                                const Ice::Exception& exc
#ifdef ICEE_HAS_AMI
                                                , const GetEndpointsCallbackPtr& callback
#endif
    )
{
    assert(ref->isIndirect());

    try
    {
        exc.ice_throw();
    }
    catch(const AdapterNotFoundException&)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "adapter not found" << "\n";
            out << "adapter = " << ref->getAdapterId();
        }

        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object adapter";
        ex.id = ref->getAdapterId();
#ifdef ICEE_HAS_AMI
        callback->locatorInfoException(ex);
#else
        throw ex;
#endif
    }
    catch(const ObjectNotFoundException&)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "object not found" << "\n";
            out << "object = " << ref->getInstance()->identityToString(ref->getIdentity());
        }

        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object";
        ex.id = ref->getInstance()->identityToString(ref->getIdentity());
#ifdef ICEE_HAS_AMI
        callback->locatorInfoException(ex);
#else
        throw ex;
#endif
    }
#ifdef ICEE_HAS_AMI
    catch(const NotRegisteredException& ex)
    {
        callback->locatorInfoException(ex);
#else
    catch(const NotRegisteredException&)
    {
        throw;
#endif
    }
    catch(const LocalException& ex)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "couldn't contact the locator to retrieve adapter endpoints\n";
            if(ref->getAdapterId().empty())
            {
                out << "object = " << ref->getInstance()->identityToString(ref->getIdentity()) << "\n";
            }
            else
            {
                out << "adapter = " << ref->getAdapterId() << "\n";
            }
            out << "reason = " << ex.toString();
        }
#ifdef ICEE_HAS_AMI
        callback->locatorInfoException(ex);
#else
        throw;
#endif
    }
}

#ifdef ICEE_HAS_AMI
void
IceInternal::LocatorInfo::getWellKnownObjectEndpoints(const ReferencePtr& ref,
                                                      const Ice::ObjectPrx& object,
                                                      bool objectCached,
                                                      const GetEndpointsCallbackPtr& callback)
{
    vector<EndpointPtr> endpoints;
    if(object)
    {
        ReferencePtr r = object->__reference();
        if(!r->isIndirect())
        {
            endpoints = r->getEndpoints();
        }
        else if(!r->isWellKnown())
        {
            getEndpoints(r, new WellKnownObjectEndpoints(this, _table, ref, object, objectCached, callback));
            return;
        }
    }

    if(!objectCached && !endpoints.empty())
    {
        _table->addProxy(ref->getIdentity(), object);
    }
    
    if(ref->getInstance()->traceLevels()->location >= 1)
    {
        getEndpointsTrace(ref, endpoints, objectCached);
    }
    
    callback->locatorInfoEndpoints(endpoints, objectCached);
}
#endif

void
IceInternal::LocatorInfo::getEndpointsTrace(const ReferencePtr& ref,
                                            const vector<EndpointPtr>& endpoints,
                                            bool cached)
{
    if(!endpoints.empty())
    {
        if(cached)
        {
            trace("found endpoints in locator table", ref, endpoints);
        }
        else
        {
            trace("retrieved endpoints from locator, adding to locator table", ref, endpoints);
        }
    }
    else
    {
        Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
        out << "no endpoints configured for ";
        if(ref->getAdapterId().empty())
        {
            out << "object\n";
            out << "object = " << ref->getInstance()->identityToString(ref->getIdentity());
        }
        else
        {
            out << "adapter\n";
            out << "adapter = " << ref->getAdapterId();
        }
    }
}

#endif
