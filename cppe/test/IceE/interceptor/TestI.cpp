// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INTERCEPTOR_TEST_API_EXPORTS
#   define INTERCEPTOR_TEST_API_EXPORTS
#endif
#include <IceE/IceE.h>
#include <Test.h>
#include <iostream>

using namespace IceUtil; 
using namespace std;

string
Test::RetryException::toString() const
{
    string out = Exception::toString();
    out += ":\nretry dispatch";
    return out;
}
