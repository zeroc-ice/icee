// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_FACTORY_MANAGER_F_H
#define ICEE_OBJECT_FACTORY_MANAGER_F_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>

namespace IceInternal
{

class ObjectFactoryManager;
IceUtil::Shared* upCast(ObjectFactoryManager*);
typedef Handle<ObjectFactoryManager> ObjectFactoryManagerPtr;

}

#endif
