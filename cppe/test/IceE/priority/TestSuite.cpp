// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Mutex.h>
#include <TestCommon.h>
#include <TestSuite.h>
#include <ThreadPriorityTest.h>
#include <TimerPriorityTest.h>
#include <PriorityInversion.h>

std::list<TestBasePtr> allTests;

void
initializeTestSuite()
{
    allTests.push_back(new ThreadPriorityTest);
    allTests.push_back(new TimerPriorityTest);
#if defined(_WIN32_WCE) || (!defined(_WIN32) && defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT > 0)
    allTests.push_back(new PriorityInversionTest);
#elif defined(_WIN32)
    tprintf("priority inversion test not supported on Windows\n");
#else
    tprintf("_POSIX_THREAD_PRIO_INHERIT not defined, skipping priority inversion test\n");
#endif
}
