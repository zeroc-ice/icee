// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NESTED_ICE
#define NESTED_ICE

module Demo
{

interface Nested
{
    void nestedCall(int level, Nested* proxy);
};

};

#endif
