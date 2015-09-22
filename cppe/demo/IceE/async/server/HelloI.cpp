// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/Thread.h>
#include <HelloI.h>

using namespace std;

void
HelloI::sayHello(int delay, const Ice::Current&)
{
    if(delay > 0)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(delay));
    }
    printf("Hello World!\n"); fflush(stdout);
}

void
HelloI::shutdown(const Ice::Current& curr)
{
    printf("Shutting down...\n");
    curr.adapter->getCommunicator()->shutdown();
}
