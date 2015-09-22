// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONNECTION_H
#define ICEE_CONNECTION_H

#include <IceE/ConnectionF.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/Proxy.h>
#    include <IceE/ObjectAdapterF.h>
#    include <IceE/Identity.h>
#endif

namespace Ice
{

class ICE_API Connection : virtual public ::IceUtil::Shared
{
public:

    virtual void close(bool) = 0;

#ifdef ICEE_HAS_BATCH
    virtual void flushBatchRequests() = 0;
#endif

#ifndef ICEE_PURE_CLIENT
    virtual void setAdapter(const ObjectAdapterPtr&) = 0;
    virtual ObjectAdapterPtr getAdapter() const = 0;
    virtual ObjectPrx createProxy(const Identity& ident) const = 0;
#endif

    virtual std::string type() const = 0;
    virtual Ice::Int timeout() const = 0;
    virtual std::string toString() const = 0;
};

}

#endif
