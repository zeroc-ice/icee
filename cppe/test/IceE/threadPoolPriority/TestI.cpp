// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace Test;
using namespace std;

PriorityI::PriorityI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
PriorityI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
#ifdef _WIN32_WCE
    tprintf("The server has shutdown, close the window to terminate the server.");
#endif
}

int
PriorityI::getPriority(const Ice::Current&)
{
#ifdef _WIN32_WCE
    return CeGetThreadPriority(GetCurrentThread());
#elif defined _WIN32
    return GetThreadPriority(GetCurrentThread());
#else
    sched_param param;
    int sched_policy;
    pthread_t thread = pthread_self();
    pthread_getschedparam(thread, &sched_policy, &param);
    return param.sched_priority;
#endif
}
