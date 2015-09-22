// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INCOMING_REQUEST_H
#define ICEE_INCOMING_REQUEST_H

#include <IceE/Incoming.h>
#include <IceE/Object.h>

namespace IceInternal
{

//
// Adapts Incoming to Ice::Request
// (the goal here is to avoid adding any virtual function to Incoming)
//
class ICE_API IncomingRequest : public Ice::Request
{
public:

    IncomingRequest(Incoming& in) :
        _in(in)
    {
    }

    virtual const Ice::Current& getCurrent();

    Incoming& _in;
};

}

#endif
