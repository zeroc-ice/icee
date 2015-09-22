// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>
#include <iostream>

using namespace std;

Test::PriorityPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing server priority... ");
    string ref = "test:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);

    Test::PriorityPrx priority = Test::PriorityPrx::checkedCast(base);

    try
    {
#ifdef _WIN32_WCE
        test(0 == priority->getPriority());
#elif defined _WIN32
        test(1 == priority->getPriority());
#else
        test(50 == priority->getPriority());
#endif
    }
    catch(...)
    {
        test(false);
    }
    tprintf("ok\n");
    return priority;
}

