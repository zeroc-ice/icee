// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DISPATCH_INTERCEPTOR_H
#define ICE_DISPATCH_INTERCEPTOR_H

#include <IceE/Object.h>

namespace Ice
{

class ICE_API DispatchInterceptor : public virtual Object
{
public:

    virtual DispatchStatus dispatch(Request&) = 0;

    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
};

typedef IceInternal::Handle<DispatchInterceptor> DispatchInterceptorPtr;

}

#endif
