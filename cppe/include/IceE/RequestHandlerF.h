// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REQUEST_HANDLER_F_H
#define ICEE_REQUEST_HANDLER_F_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>

namespace IceInternal
{

class RequestHandler;
ICE_API IceUtil::Shared* upCast(RequestHandler*);
typedef IceInternal::Handle<RequestHandler> RequestHandlerPtr;

}

#endif
