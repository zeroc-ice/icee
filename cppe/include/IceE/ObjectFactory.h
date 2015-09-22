// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_FACTORY_H
#define ICEE_OBJECT_FACTORY_H

#include <IceE/ObjectFactoryF.h>
#include <IceE/ObjectF.h>

namespace Ice
{

class ICE_API ObjectFactory : public ::IceUtil::Shared
{
public:
    
    virtual ::Ice::ObjectPtr create(const ::std::string&) = 0;
    virtual void destroy() = 0;
};


}

#endif
