// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_FACTORY_F_H
#define ICEE_OBJECT_FACTORY_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace Ice
{

class ObjectFactory;
inline bool operator==(const ObjectFactory& l, const ObjectFactory& r)
{
    return &l == &r;
}
inline bool operator<(const ObjectFactory& l, const ObjectFactory& r)
{
    return &l < &r;
}

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::ObjectFactory*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::ObjectFactory> ObjectFactoryPtr;

}

#endif
