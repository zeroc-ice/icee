// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <MyObjectFactory.h>
#include <TestI.h>

using namespace std;

Ice::ObjectPtr 
MyObjectFactory::create(const string& type)
{
    if(type == "::Test::B")
    {
        return new BI;
    }
    else if(type == "::Test::C")
    {
        return new CI;
    }
    else if(type == "::Test::D")
    {
        return new DI;
    }
    else if(type == "::Test::E")
    {
        return new EI;
    }
    else if(type == "::Test::F")
    {
        return new FI;
    }
    else if(type == "::Test::I")
    {
        return new II;
    }
    else if(type == "::Test::J")
    {
        return new JI;
    }
    else if(type == "::Test::H")
    {
        return new HI;
    }

    assert(false); // Should never be reached
    return 0;
}

void
MyObjectFactory::destroy()
{
    // Nothing to do
}
