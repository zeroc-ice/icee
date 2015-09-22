// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_MUTEX_PROTOCOL_H
#define ICEE_MUTEX_PROTOCOL_H

#include <IceE/Config.h>

namespace IceUtil
{

enum MutexProtocol
{
    PrioNone,
    PrioInherit
};

ICE_API MutexProtocol getDefaultMutexProtocol();

} // End namespace IceUtil

#endif
