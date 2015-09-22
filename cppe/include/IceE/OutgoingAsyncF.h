// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_ASYNC_F_H
#define ICEE_OUTGOING_ASYNC_F_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>

#ifdef ICEE_HAS_AMI

namespace IceInternal
{

class OutgoingAsync;
ICE_API IceUtil::Shared* upCast(OutgoingAsync*);
typedef IceInternal::Handle<OutgoingAsync> OutgoingAsyncPtr;

class OutgoingAsyncMessageCallback;
ICE_API IceUtil::Shared* upCast(OutgoingAsyncMessageCallback*);
typedef IceInternal::Handle<OutgoingAsyncMessageCallback> OutgoingAsyncMessageCallbackPtr;

#ifdef ICEE_HAS_BATCH
class BatchOutgoingAsync;
ICE_API IceUtil::Shared* upCast(BatchOutgoingAsync*);
typedef IceInternal::Handle<BatchOutgoingAsync> BatchOutgoingAsyncPtr;
#endif

}

#ifdef ICEE_HAS_BATCH
namespace Ice
{

class AMI_Object_ice_flushBatchRequests;

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::AMI_Object_ice_flushBatchRequests*);

}

namespace Ice
{

typedef IceInternal::Handle<AMI_Object_ice_flushBatchRequests> AMI_Object_ice_flushBatchRequestsPtr;

}
#endif

#endif

#endif
