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
#include <ClientPrivate.h>

using namespace std;
using namespace Test;

TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectPrx obj = communicator->stringToProxy("Test:default -p 12010");
    TestIntfPrx test = TestIntfPrx::checkedCast(obj);

    tprintf("base as Object... ");
    {
        Ice::ObjectPtr o;
        try
        {
            o = test->SBaseAsObject();
            test(o);
            test(o->ice_id() == "::Test::SBase");
        }
        catch(...)
        {
            test(0);
        }
        SBasePtr sb = SBasePtr::dynamicCast(o);
        test(sb);
        test(sb->sb == "SBase.sb");
    }
    tprintf("ok\n");

    tprintf("base as base... ");
    {
        SBasePtr sb;
        try
        {
            sb = test->SBaseAsSBase();
            test(sb->sb == "SBase.sb");
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("base with known derived as base... ");
    {
        SBasePtr sb;
        try
        {
            sb = test->SBSKnownDerivedAsSBase();
            test(sb->sb == "SBSKnownDerived.sb");
        }
        catch(...)
        {
            test(0);
        }
        SBSKnownDerivedPtr sbskd = SBSKnownDerivedPtr::dynamicCast(sb);
        test(sbskd);
        test(sbskd->sbskd == "SBSKnownDerived.sbskd");
    }
    tprintf("ok\n");

    tprintf("base with known derived as known derived... ");
    {
        SBSKnownDerivedPtr sbskd;
        try
        {
            sbskd = test->SBSKnownDerivedAsSBSKnownDerived();
            test(sbskd->sbskd == "SBSKnownDerived.sbskd");
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("base with unknown derived as base... ");
    {
        SBasePtr sb;
        try
        {
            sb = test->SBSUnknownDerivedAsSBase();
            test(sb->sb == "SBSUnknownDerived.sb");
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("unknown with Object as Object... ");
    {
        Ice::ObjectPtr o;
        try
        {
            o = test->SUnknownAsObject();
            test(0);
        }
        catch(const Ice::MarshalException&)
        {
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("one-element cycle... ");
    {
        try
        {
            BPtr b = test->oneElementCycle();
            test(b);
            test(b->ice_id() == "::Test::B");
            test(b->sb == "B1.sb");
            test(b->pb == b);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("two-element cycle... ");
    {
        try
        {
            BPtr b1 = test->twoElementCycle();
            test(b1);
            test(b1->ice_id() == "::Test::B");
            test(b1->sb == "B1.sb");

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "B2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("known derived pointer slicing as base... ");
    {
        try
        {
            BPtr b1;
            b1 = test->D1AsB();
            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb);
            test(b1->pb != b1);
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1);
            test(d1->pd1 != b1);
            test(b1->pb == d1->pd1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->pb == b1);
            test(b2->sb == "D2.sb");
            test(b2->ice_id() == "::Test::B");
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("known derived pointer slicing as derived... ");
    {
        try
        {
            D1Ptr d1;
            d1 = test->D1AsD1();
            test(d1);
            test(d1->ice_id() == "::Test::D1");
            test(d1->sb == "D1.sb");
            test(d1->pb);
            test(d1->pb != d1);

            BPtr b2 = d1->pb;
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb == d1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("unknown derived pointer slicing as base... ");
    {
        try
        {
            BPtr b2;
            b2 = test->D2AsB();
            test(b2);
            test(b2->ice_id() == "::Test::B");
            test(b2->sb == "D2.sb");
            test(b2->pb);
            test(b2->pb != b2);

            BPtr b1 = b2->pb;
            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("param ptr slicing with known first... ");
    {
        try
        {
            BPtr b1;
            BPtr b2;
            test->paramTest1(b1, b2);

            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(b2->ice_id() == "::Test::B");  // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("param ptr slicing with unknown first... ");
    {
        try
        {
            BPtr b2;
            BPtr b1;
            test->paramTest2(b2, b1);

            test(b1);
            test(b1->ice_id() == "::Test::D1");
            test(b1->sb == "D1.sb");
            test(b1->pb == b2);
            D1Ptr d1 = D1Ptr::dynamicCast(b1);
            test(d1);
            test(d1->sd1 == "D1.sd1");
            test(d1->pd1 == b2);

            test(b2);
            test(b2->ice_id() == "::Test::B");  // No factory, must be sliced
            test(b2->sb == "D2.sb");
            test(b2->pb == b1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("return value identity with known first... ");
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr r = test->returnTest1(p1, p2);
            test(r == p1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("return value identity with unknown first... ");
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr r = test->returnTest2(p1, p2);
            test(r == p1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("return value identity for input params known first... ");
    {
        try
        {
            D1Ptr d1 = new D1;
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = new D3;
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;

            BPtr b1 = test->returnTest3(d1, d3);

            test(b1);
            test(b1->sb == "D1.sb");
            test(b1->ice_id() == "::Test::D1");
            D1Ptr p1 = D1Ptr::dynamicCast(b1);
            test(p1);
            test(p1->sd1 == "D1.sd1");
            test(p1->pd1 == b1->pb);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D3.sb");
            test(b2->ice_id() == "::Test::B");  // Sliced by server
            test(b2->pb == b1);
            D3Ptr p3 = D3Ptr::dynamicCast(b2);
            test(!p3);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("return value identity for input params unknown first... ");
    {
        try
        {
            D1Ptr d1 = new D1;
            d1->sb = "D1.sb";
            d1->sd1 = "D1.sd1";
            D3Ptr d3 = new D3;
            d3->pb = d1;
            d3->sb = "D3.sb";
            d3->sd3 = "D3.sd3";
            d3->pd3 = d1;
            d1->pb = d3;
            d1->pd1 = d3;

            BPtr b1 = test->returnTest3(d3, d1);

            test(b1);
            test(b1->sb == "D3.sb");
            test(b1->ice_id() == "::Test::B");  // Sliced by server
            D3Ptr p1 = D3Ptr::dynamicCast(b1);
            test(!p1);

            BPtr b2 = b1->pb;
            test(b2);
            test(b2->sb == "D1.sb");
            test(b2->ice_id() == "::Test::D1");
            test(b2->pb == b1);
            D1Ptr p3 = D1Ptr::dynamicCast(b2);
            test(p3);
            test(p3->sd1 == "D1.sd1");
            test(p3->pd1 == b1);

            test(b1 != d1);
            test(b1 != d3);
            test(b2 != d1);
            test(b2 != d3);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("remainder unmarshaling (3 instances)... ");
    {
        try
        {
            BPtr p1;
            BPtr p2;
            BPtr ret = test->paramTest3(p1, p2);

            test(p1);
            test(p1->sb == "D2.sb (p1 1)");
            test(p1->pb == 0);
            test(p1->ice_id() == "::Test::B");

            test(p2);
            test(p2->sb == "D2.sb (p2 1)");
            test(p2->pb == 0);
            test(p2->ice_id() == "::Test::B");

            test(ret);
            test(ret->sb == "D1.sb (p2 2)");
            test(ret->pb == 0);
            test(ret->ice_id() == "::Test::D1");
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("remainder unmarshaling (4 instances)... ");
    {
        try
        {
            BPtr b;
            BPtr ret = test->paramTest4(b);

            test(b);
            test(b->sb == "D4.sb (1)");
            test(b->pb == 0);
            test(b->ice_id() == "::Test::B");

            test(ret);
            test(ret->sb == "B.sb (2)");
            test(ret->pb == 0);
            test(ret->ice_id() == "::Test::B");
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("param ptr slicing, instance marshaled in unknown derived as base... ");
    {
        try
        {
            BPtr b1 = new B;
            b1->sb = "B.sb(1)";
            b1->pb = b1;

            D3Ptr d3 = new D3;
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = b1;

            BPtr b2 = new B;
            b2->sb = "B.sb(2)";
            b2->pb = b1;

            BPtr r = test->returnTest3(d3, b2);

            test(r);
            test(r->ice_id() == "::Test::B");
            test(r->sb == "D3.sb");
            test(r->pb == r);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("param ptr slicing, instance marshaled in unknown derived as derived... "
        );
    {
        try
        {
            D1Ptr d11 = new D1;
            d11->sb = "D1.sb(1)";
            d11->pb = d11;
            d11->sd1 = "D1.sd1(1)";

            D3Ptr d3 = new D3;
            d3->sb = "D3.sb";
            d3->pb = d3;
            d3->sd3 = "D3.sd3";
            d3->pd3 = d11;

            D1Ptr d12 = new D1;
            d12->sb = "D1.sb(2)";
            d12->pb = d12;
            d12->sd1 = "D1.sd1(2)";
            d12->pd1 = d11;

            BPtr r = test->returnTest3(d3, d12);
            test(r);
            test(r->ice_id() == "::Test::B");
            test(r->sb == "D3.sb");
            test(r->pb == r);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("sequence slicing... ");
    {
        try
        {
            SS ss;
            {
                BPtr ss1b = new B;
                ss1b->sb = "B.sb";
                ss1b->pb = ss1b;

                D1Ptr ss1d1 = new D1;
                ss1d1->sb = "D1.sb";
                ss1d1->sd1 = "D1.sd1";
                ss1d1->pb = ss1b;

                D3Ptr ss1d3 = new D3;
                ss1d3->sb = "D3.sb";
                ss1d3->sd3 = "D3.sd3";
                ss1d3->pb = ss1b;

                BPtr ss2b = new B;
                ss2b->sb = "B.sb";
                ss2b->pb = ss1b;

                D1Ptr ss2d1 = new D1;
                ss2d1->sb = "D1.sb";
                ss2d1->sd1 = "D1.sd1";
                ss2d1->pb = ss2b;

                D3Ptr ss2d3 = new D3;
                ss2d3->sb = "D3.sb";
                ss2d3->sd3 = "D3.sd3";
                ss2d3->pb = ss2b;

                ss1d1->pd1 = ss2b;
                ss1d3->pd3 = ss2d1;

                ss2d1->pd1 = ss1d3;
                ss2d3->pd3 = ss1d1;

                SS1Ptr ss1 = new SS1;
                ss1->s.push_back(ss1b);
                ss1->s.push_back(ss1d1);
                ss1->s.push_back(ss1d3);

                SS2Ptr ss2 = new SS2;
                ss2->s.push_back(ss2b);
                ss2->s.push_back(ss2d1);
                ss2->s.push_back(ss2d3);

                ss = test->sequenceTest(ss1, ss2);
            }

            test(ss.c1);
            BPtr ss1b = ss.c1->s[0];
            BPtr ss1d1 = ss.c1->s[1];
            test(ss.c2);
            BPtr ss1d3 = ss.c1->s[2];

            test(ss.c2);
            BPtr ss2b = ss.c2->s[0];
            BPtr ss2d1 = ss.c2->s[1];
            BPtr ss2d3 = ss.c2->s[2];

            test(ss1b->pb == ss1b);
            test(ss1d1->pb == ss1b);
            test(ss1d3->pb == ss1b);

            test(ss2b->pb == ss1b);
            test(ss2d1->pb == ss2b);
            test(ss2d3->pb == ss2b);

            test(ss1b->ice_id() == "::Test::B");
            test(ss1d1->ice_id() == "::Test::D1");
            test(ss1d3->ice_id() == "::Test::B");

            test(ss2b->ice_id() == "::Test::B");
            test(ss2d1->ice_id() == "::Test::D1");
            test(ss2d3->ice_id() == "::Test::B");
        }
        catch(const ::Ice::Exception&)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("dictionary slicing... ");
    {
        try
        {
            BDict bin;
            BDict bout;
            BDict r;
            int i;
            for(i = 0; i < 10; ++i)
            {
                string s = Ice::printfToString("D1.%d", i);
                D1Ptr d1 = new D1;
                d1->sb = s;
                d1->pb = d1;
                d1->sd1 = s;
                bin[i] = d1;
            }

            r = test->dictionaryTest(bin, bout);

            test(bout.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                BPtr b = bout.find(i * 10)->second;
                test(b);
                string s = Ice::printfToString("D1.%d", i);
                test(b->sb == s);
                test(b->pb);
                test(b->pb != b);
                test(b->pb->sb == s);
                test(b->pb->pb == b->pb);
            }

            test(r.size() == 10);
            for(i = 0; i < 10; ++i)
            {
                BPtr b = r.find(i * 20)->second;
                test(b);
                string s = Ice::printfToString("D1.%d", i * 20);
                test(b->sb == s);
                test(b->pb == (i == 0 ? BPtr(0) : r.find((i - 1) * 20)->second));
                D1Ptr d1 = D1Ptr::dynamicCast(b);
                test(d1);
                test(d1->sd1 == s);
                test(d1->pd1 == d1);
            }
        }
        catch(const ::Ice::Exception&)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("base exception thrown as base exception... ");
    {
        try
        {
            test->throwBaseAsBase();
            test(0);
        }
        catch(const BaseException& e)
        {
            test(e.ice_name() == "Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb");
            test(e.pb->pb == e.pb);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("derived exception thrown as base exception... ");
    {
        try
        {
            test->throwDerivedAsBase();
            test(0);
        }
        catch(const DerivedException& e)
        {
            test(e.ice_name() == "Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("derived exception thrown as derived exception... ");
    {
        try
        {
            test->throwDerivedAsDerived();
            test(0);
        }
        catch(const DerivedException& e)
        {
            test(e.ice_name() == "Test::DerivedException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb1");
            test(e.pb->pb == e.pb);
            test(e.sde == "sde1");
            test(e.pd1);
            test(e.pd1->sb == "sb2");
            test(e.pd1->pb == e.pd1);
            test(e.pd1->sd1 == "sd2");
            test(e.pd1->pd1 == e.pd1);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("unknown derived exception thrown as base exception... ");
    {
        try
        {
            test->throwUnknownDerivedAsBase();
            test(0);
        }
        catch(const BaseException& e)
        {
            test(e.ice_name() == "Test::BaseException");
            test(e.sbe == "sbe");
            test(e.pb);
            test(e.pb->sb == "sb d2");
            test(e.pb->pb == e.pb);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    tprintf("forward-declared class... ");
    {
        try
        {
            ForwardPtr f;
            test->useForward(f);
            test(f);
        }
        catch(...)
        {
            test(0);
        }
    }
    tprintf("ok\n");

    return test;
}
