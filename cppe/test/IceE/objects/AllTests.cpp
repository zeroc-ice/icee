// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

InitialPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = "initial:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    InitialPrx initial = InitialPrx::checkedCast(base);
    test(initial);
    test(initial == base);
    tprintf("ok\n");

    tprintf("testing constructor, copy constructor, and assignment operator... ");

    BasePtr ba1 = new Base;
    test(ba1->theS.str == "");
    test(ba1->str == "");

    S s;
    s.str = "hello";
    BasePtr ba2 = new Base(s, "hi");
    test(ba2->theS.str == "hello");
    test(ba2->str == "hi");

    *ba1 = *ba2;
    test(ba1->theS.str == "hello");
    test(ba1->str == "hi");

    BasePtr bp1 = new Base();
    *bp1 = *ba2;
    test(bp1->theS.str == "hello");
    test(bp1->str == "hi");

    tprintf("ok\n");

    tprintf("getting B1... ");
    BPtr b1 = initial->getB1();
    test(b1);
    tprintf("ok\n");
    
    tprintf("getting B2... ");
    BPtr b2 = initial->getB2();
    test(b2);
    tprintf("ok\n");
    
    tprintf("getting C... ");
    CPtr c = initial->getC();
    test(c);
    tprintf("ok\n");
    
    tprintf("getting D... ");
    DPtr d = initial->getD();
    test(d);
    tprintf("ok\n");
    
    tprintf("checking consistency... ");
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    test(b1->theB == b1);
    test(b1->theC == 0);
    test(BPtr::dynamicCast(b1->theA));
    test(BPtr::dynamicCast(b1->theA)->theA == b1->theA);
    test(BPtr::dynamicCast(b1->theA)->theB == b1);
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->theA)->theC));
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->theA)->theC)->theB == b1->theA);
    test(b1->preMarshalInvoked);
    test(b1->postUnmarshalInvoked());
    test(b1->theA->preMarshalInvoked);
    test(b1->theA->postUnmarshalInvoked());
    test(BPtr::dynamicCast(b1->theA)->theC->preMarshalInvoked);
    test(BPtr::dynamicCast(b1->theA)->theC->postUnmarshalInvoked());
    // More tests possible for b2 and d, but I think this is already sufficient.
    test(b2->theA == b2);
    test(d->theC == 0);
    tprintf("ok\n");

    tprintf("getting B1, B2, C, and D all at once... ");
    initial->getAll(b1, b2, c, d);
    test(b1);
    test(b2);
    test(c);
    test(d);
    tprintf("ok\n");
    
    tprintf("checking consistency... ");
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    test(b1->theA == b2);
    test(b1->theB == b1);
    test(b1->theC == 0);
    test(b2->theA == b2);
    test(b2->theB == b1);
    test(b2->theC == c);
    test(c->theB == b2);
    test(d->theA == b1);
    test(d->theB == b2);
    test(d->theC == 0);
    test(d->preMarshalInvoked);
    test(d->postUnmarshalInvoked());
    test(d->theA->preMarshalInvoked);
    test(d->theA->postUnmarshalInvoked());
    test(d->theB->preMarshalInvoked);
    test(d->theB->postUnmarshalInvoked());
    test(d->theB->theC->preMarshalInvoked);
    test(d->theB->theC->postUnmarshalInvoked());
    tprintf("ok\n");

    tprintf("testing protected members... ");
    EPtr e = initial->getE();
    test(e->checkValues());
    FPtr f = initial->getF();
    test(f->checkValues());
    test(f->e2->checkValues());
    tprintf("ok\n");

    tprintf("getting I, J and H... ");
    IPtr i = initial->getI();
    test(i);
    IPtr j = initial->getJ();
    test(j && JPtr::dynamicCast(j));
    IPtr h = initial->getH();
    test(h && HPtr::dynamicCast(h));
    tprintf("ok\n");

    tprintf("setting I... ");
    initial->setI(i);
    initial->setI(j);
    initial->setI(h);
    tprintf("ok\n");

    return initial;
}
