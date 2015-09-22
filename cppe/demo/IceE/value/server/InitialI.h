// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INITIAL_I_H
#define INITIAL_I_H

#include <Value.h>

class InitialI : public Demo::Initial
{
public:

    InitialI(const Ice::ObjectAdapterPtr&);

    virtual Demo::SimplePtr getSimple(const Ice::Current&);
    virtual void getPrinter(::Demo::PrinterPtr&, Demo::PrinterPrx&, const Ice::Current&);
    virtual Demo::PrinterPtr getDerivedPrinter(const Ice::Current&);
    virtual void throwDerivedPrinter(const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    const Demo::SimplePtr _simple;
    const Demo::PrinterPtr _printer;
    const Demo::PrinterPrx _printerProxy;
    const Demo::DerivedPrinterPtr _derivedPrinter;
};

#endif
