// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#include <IceE/ObjectFactoryManagerI.h>
#include <IceE/ObjectFactory.h>
#include <IceE/Functional.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::ObjectFactoryManagerI::add(const ObjectFactoryPtr& factory, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    if((_factoryMapHint != _factoryMap.end() && _factoryMapHint->first == id)
       || _factoryMap.find(id) != _factoryMap.end())
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "object factory";
        ex.id = id;
        throw ex;
    }

    _factoryMapHint = _factoryMap.insert(_factoryMapHint, pair<const string, ObjectFactoryPtr>(id, factory));
}

ObjectFactoryPtr
IceInternal::ObjectFactoryManagerI::find(const string& id) const
{
    IceUtil::Mutex::Lock sync(*this);
   
    FactoryMap& factoryMap = const_cast<FactoryMap&>(_factoryMap);

    FactoryMap::iterator p = factoryMap.end();
    if(_factoryMapHint != factoryMap.end())
    {
        if(_factoryMapHint->first == id)
        {
            p = _factoryMapHint;
        }
    }
    
    if(p == factoryMap.end())
    {
        p = factoryMap.find(id);
    }
    
    if(p != factoryMap.end())
    {
        _factoryMapHint = p;
        return p->second;
    }
    else
    {
        return 0;
    }
}

IceInternal::ObjectFactoryManagerI::ObjectFactoryManagerI() :
    _factoryMapHint(_factoryMap.end())
{
}

void
IceInternal::ObjectFactoryManagerI::destroy()
{
    FactoryMap oldMap;
    {
        IceUtil::Mutex::Lock sync(*this);
        oldMap.swap(_factoryMap);
        _factoryMapHint = _factoryMap.end();
    }

    //
    // Destroy all outside lock
    //
    for_each(oldMap.begin(), oldMap.end(),
             Ice::secondVoidMemFun<const string, ObjectFactory>(&ObjectFactory::destroy));
}

