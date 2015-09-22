// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Thread.h>

#include <ThreadPriorityTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

class PriorityTestThread : public Thread
{
public:
    
    PriorityTestThread() :
        _priority(-1024) // Initialize to a non-valid priority.
    {
    }

    virtual void run()
    {
        ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
#ifdef _WIN32_WCE
        _priority = CeGetThreadPriority(GetCurrentThread());
#elif defined _WIN32
        _priority = GetThreadPriority(GetCurrentThread());
#else
        sched_param param;
        int sched_policy;
        pthread_t thread = pthread_self();
        pthread_getschedparam(thread, &sched_policy, &param);
        _priority = param.sched_priority;
#endif
    }

    int getPriority()
    {
        return _priority;
    }

private:

    int _priority;
};

typedef Handle<PriorityTestThread> PriorityTestThreadPtr;

static const string priorityTestName("thread priority");

ThreadPriorityTest::ThreadPriorityTest() :
    TestBase(priorityTestName)
{
}

void
ThreadPriorityTest::run()
{
#ifdef _WIN32_WCE
    //
    // Test to start a thread with a given priority
    //
    ThreadControl c;
    try
    {
        for(int cont = 0; cont < 255; ++cont)
        {
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, cont);
            c.join();
            test(t1->getPriority() == cont);
        }
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set priorities too high
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            int pri = 300 * cont;
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, pri);
            test(false);
        }
        catch(const ThreadSyscallException&)
        {
            //Expected
        }
        catch(...)
        {
            test(false);
        }
    }

    //
    // Test to set invalid priorities too low
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            int pri = -10 * cont;
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, pri);
            test(false);
        }
        catch(const ThreadSyscallException&)
        {
            //Expected
        }
        catch(...)
        {
            test(false);
        }
    }

#elif defined _WIN32
    //
    // Test to start a thread with a given priority
    //
    ThreadControl c;
    try
    {
        PriorityTestThreadPtr t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_IDLE);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_IDLE);

        t1 = new PriorityTestThread();
        t1->start(128, THREAD_PRIORITY_LOWEST);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_LOWEST);

        t1 = new PriorityTestThread();
        t1->start(128, THREAD_PRIORITY_BELOW_NORMAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_BELOW_NORMAL);

        t1 = new PriorityTestThread();
        t1->start(128, THREAD_PRIORITY_NORMAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_NORMAL);

        t1 = new PriorityTestThread();
        t1->start(128, THREAD_PRIORITY_ABOVE_NORMAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_ABOVE_NORMAL);

        t1 = new PriorityTestThread();
        t1->start(128, THREAD_PRIORITY_HIGHEST);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_HIGHEST);

        t1 = new PriorityTestThread();
        t1->start(128, THREAD_PRIORITY_TIME_CRITICAL);
        c.join();
        test(t1->getPriority() == THREAD_PRIORITY_TIME_CRITICAL);
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set priorities too high
    //
    try
    {
        PriorityTestThreadPtr t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_TIME_CRITICAL + 10);
        test(false);
    }
    catch(const ThreadSyscallException&)
    {
        //Expected
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set priorities too low
    //
    try
    {
        PriorityTestThreadPtr t1 = new PriorityTestThread();
        c = t1->start(128, THREAD_PRIORITY_IDLE - 10);
        test(false);
    }
    catch(const ThreadSyscallException&)
    {
        //Expected
    }
    catch(...)
    {
        test(false);
    }

#else

    //
    // Test to start a thread with a given priority
    //
    ThreadControl c;
    try
    {
        for(int cont = 1; cont < 10; ++cont)
        {
            int pri = 10 * cont;
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, pri);
            c.join();
            test(t1->getPriority() == pri);
        }
    }
    catch(...)
    {
        test(false);
    }

    //
    // Test to set priorities too high
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            int pri = 300 * cont;
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, pri);
            test(false);
        }
        catch(const ThreadSyscallException& e)
        {
            //Expected
        }
        catch(...)
        {
            test(false);
        }
    }

    //
    // Test to set priorities too low
    //
    for(int cont = 1; cont < 10; ++cont)
    {
        try
        {
            int pri = -10 * cont;
            PriorityTestThreadPtr t1 = new PriorityTestThread();
            c = t1->start(128, pri);
            test(false);
        }
        catch(const ThreadSyscallException& e)
        {
            //Expected
        }
        catch(...)
        {
            test(false);
        }
    }
#endif
}

