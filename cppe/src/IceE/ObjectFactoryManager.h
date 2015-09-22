// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_FACTORY_MANAGER_H
#define ICEE_OBJECT_FACTORY_MANAGER_H

#include <IceE/Shared.h>
#include <IceE/ObjectFactoryManagerF.h>
#include <IceE/ObjectFactoryF.h>

namespace IceInternal
{

//
// Code size optimization: we define this abstract class to allow the linker
// to not import its implementation if it no user object factories are used
// by statically linked executables.
//
class ObjectFactoryManager : public ::IceUtil::Shared
{
public:

    virtual void add(const ::Ice::ObjectFactoryPtr&, const std::string&) = 0;
    virtual ::Ice::ObjectFactoryPtr find(const std::string&) const = 0;
    virtual void destroy() = 0;
};

}

#endif
