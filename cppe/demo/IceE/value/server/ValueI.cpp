// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <ValueI.h>

using namespace std;
using namespace Demo;

void
PrinterI::printBackwards(const Ice::Current&)
{
    string s;
    s.resize(message.length());
    reverse_copy(message.begin(), message.end(), s.begin());
    printf("%s\n", s.c_str()); fflush(stdout);
}

void
DerivedPrinterI::printUppercase(const Ice::Current&)
{
    string s;
    s.resize(derivedMessage.length());
    transform(derivedMessage.begin(), derivedMessage.end(), s.begin(), ::toupper);
    printf("%s\n", s.c_str()); fflush(stdout);
}
