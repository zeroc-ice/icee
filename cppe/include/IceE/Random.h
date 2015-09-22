// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_UTIL_RANDOM_H
#define ICEE_UTIL_RANDOM_H

#include <IceE/Config.h>

namespace IceUtilInternal
{

ICE_API void generateRandom(char*, int);
ICE_API int random(int = 0);

}

#endif
