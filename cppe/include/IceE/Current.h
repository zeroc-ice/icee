// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CURRENT_H
#define ICEE_CURRENT_H

#include <IceE/Config.h>

#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapterF.h>
#endif
#include <IceE/ConnectionF.h>
#include <IceE/Identity.h>
#include <IceE/OperationMode.h>

namespace Ice
{

struct Current
{
#ifndef ICEE_PURE_CLIENT
    ::Ice::ObjectAdapter* adapter;
#endif
    ::Ice::Connection* con;
    ::Ice::Identity id;
    ::std::string facet;
    ::std::string operation;
    ::Ice::OperationMode mode;
    ::Ice::Context ctx;
    ::Ice::Int requestId;

    ICE_API bool operator==(const Current&) const;
    ICE_API bool operator!=(const Current&) const;
    ICE_API bool operator<(const Current&) const;
};

}

#endif
