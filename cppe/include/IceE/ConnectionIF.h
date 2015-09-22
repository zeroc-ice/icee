// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONNECTIONI_F_H
#define ICEE_CONNECTIONI_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace Ice
{

class ConnectionI;

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::ConnectionI*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::ConnectionI> ConnectionIPtr;

}

#endif
