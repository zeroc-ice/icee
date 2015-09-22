// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <MyObjectI.h>
#include <TestCommon.h>

using namespace IceUtil; 
using namespace std;

int 
MyObjectI::add(int x, int y, const Ice::Current&)
{
    return x + y;
}

int 
MyObjectI::addWithRetry(int x, int y, const Ice::Current& current)
{
    Ice::Context::const_iterator p = current.ctx.find("retry");
    
    if(p == current.ctx.end() || p->second != "no")
    {
        throw Test::RetryException(__FILE__, __LINE__);
    }
    return x + y;
}

int 
MyObjectI::badAdd(int, int, const Ice::Current&)
{
    throw Test::InvalidInputException();
}

int 
MyObjectI::notExistAdd(int, int, const Ice::Current&)
{
    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
}

int 
MyObjectI::badSystemAdd(int, int, const Ice::Current&)
{
    throw Ice::InitializationException(__FILE__, __LINE__, "testing");
#ifdef __SUNPRO_CC
    return 0;
#endif
}
