// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_I_H
#define HELLO_I_H

#include <Hello.h>

class HelloI : virtual public Demo::Hello
{
public:

    virtual void sayHello(int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
};

#endif
