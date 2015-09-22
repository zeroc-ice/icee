// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DispatchInterceptor.h>
#include <IceE/IncomingRequest.h>

using namespace Ice;
using namespace IceInternal;

DispatchStatus
Ice::DispatchInterceptor::__dispatch(IceInternal::Incoming& in, const Current& current)
{
    IncomingRequest request(in);
    return dispatch(request);
}
