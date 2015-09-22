// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SELECTOR_THREAD_F_H
#define ICEE_SELECTOR_THREAD_F_H

#include <IceE/Shared.h>

#include <IceE/Handle.h>

namespace IceInternal
{

class SelectorThread;
ICE_API IceUtil::Shared* upCast(SelectorThread*);
typedef Handle<SelectorThread> SelectorThreadPtr;

}

#endif
