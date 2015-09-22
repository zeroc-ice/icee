// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_FACTORY_MANAGERI_H
#define ICEE_OBJECT_FACTORY_MANAGERI_H

#include <IceE/Config.h>

#include <IceE/Mutex.h>
#include <IceE/ObjectFactoryManager.h>

namespace IceInternal
{

class ObjectFactoryManagerI : public ObjectFactoryManager, public ::IceUtil::Mutex
{
public:

    virtual void add(const ::Ice::ObjectFactoryPtr&, const std::string&);
    virtual ::Ice::ObjectFactoryPtr find(const std::string&) const;
    virtual void destroy();

private:

    ObjectFactoryManagerI();
    friend class Instance;

    typedef std::map<std::string, ::Ice::ObjectFactoryPtr> FactoryMap;
    FactoryMap _factoryMap;
    mutable FactoryMap::iterator _factoryMapHint;
};

}

#endif
