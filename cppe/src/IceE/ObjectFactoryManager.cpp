// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#include <IceE/ObjectFactoryManager.h>
#include <IceE/ObjectFactory.h>

using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ObjectFactory* p) { return p; }
IceUtil::Shared* IceInternal::upCast(ObjectFactoryManager* p) { return p; }
