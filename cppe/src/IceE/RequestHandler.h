// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_REQUEST_HANDLER_H
#define ICEE_REQUEST_HANDLER_H

#include <IceE/Shared.h>
#include <IceE/RequestHandlerF.h>
#include <IceE/ReferenceF.h>
#include <IceE/OutgoingAsyncF.h>
#include <IceE/ConnectionIF.h>

namespace IceInternal
{

class BasicStream;
class Outgoing;
#ifdef ICEE_HAS_BATCH
class BatchOutgoing;
#endif

class RequestHandler : virtual public ::IceUtil::Shared
{
public:

    virtual ~RequestHandler();

#ifdef ICEE_HAS_BATCH
    virtual void prepareBatchRequest(BasicStream*) = 0;
    virtual void finishBatchRequest(BasicStream*) = 0;
    virtual void abortBatchRequest() = 0;
#endif

    virtual Ice::ConnectionI* sendRequest(Outgoing*, bool) = 0;
#ifdef ICEE_HAS_AMI
    virtual bool sendAsyncRequest(const OutgoingAsyncPtr&, bool) = 0;
#endif

#ifdef ICEE_HAS_BATCH
    virtual bool flushBatchRequests(BatchOutgoing*) = 0;
#ifdef ICEE_HAS_AMI
    virtual bool flushAsyncBatchRequests(const BatchOutgoingAsyncPtr&) = 0;
#endif
#endif

    virtual Ice::ConnectionIPtr getConnection(bool) = 0;
};

}

#endif
