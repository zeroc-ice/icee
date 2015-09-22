// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_THREAD_POOL_F_H
#define ICEE_THREAD_POOL_F_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>

namespace IceInternal
{

class ThreadPool;
ICE_API IceUtil::Shared* upCast(ThreadPool*);
typedef Handle<ThreadPool> ThreadPoolPtr;

}

#endif
