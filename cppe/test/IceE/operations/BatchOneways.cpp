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

#if defined(ICEE_HAS_BATCH) && defined(ICEE_HAS_AMI)
class AMI_flushBatchRequestsI : public Ice::AMI_Object_ice_flushBatchRequests
{
public:

    virtual void ice_exception(const Ice::Exception& ex)
    {
        tprintf("%s\n", ex.toString().c_str());
        test(false);
    }
};
#endif


void
batchOneways(const Test::MyClassPrx& p)
{
#ifdef ICEE_HAS_BATCH
    const Test::ByteS bs1(10  * 1024, 0);
    const Test::ByteS bs2(99  * 1024, 0);
    const Test::ByteS bs3(100  * 1024, 0);
    
    try
    {
        p->opByteSOneway(bs1);
        test(true);
    }
    catch(const Ice::MemoryLimitException&)
    {
        test(false);
    }

    try
    {
        p->opByteSOneway(bs2);
        test(true);
    }
    catch(const Ice::MemoryLimitException&)
    {
        test(false);
    }
    
    try
    {
        p->opByteSOneway(bs3);
        test(false);
    }
    catch(const Ice::MemoryLimitException&)
    {
        test(true);
    }
    
    Test::MyClassPrx batch = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());
    
    int i;

    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
            test(true);
        }
        catch(const Ice::MemoryLimitException&)
        {
            test(false);
        }
    }
    
    batch->ice_getConnection()->flushBatchRequests();


#ifdef ICEE_HAS_AMI
    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
            test(true);
        }
        catch(const Ice::MemoryLimitException&)
        {
            test(false);
        }
    }
    batch->ice_flushBatchRequests_async(new AMI_flushBatchRequestsI);
#endif

    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
            test(true);
        }
        catch(const Ice::MemoryLimitException&)
        {
            test(false);
        }
    }
    batch->ice_flushBatchRequests();

#endif
}
