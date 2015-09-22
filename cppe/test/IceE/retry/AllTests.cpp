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

#ifdef ICEE_HAS_AMI
class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
        _called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    bool check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            if(!timedWait(IceUtil::Time::seconds(5)))
            {
                return false;
            }
        }
        _called = false;
        return true;
    }

protected:

    void called()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(!_called);
        _called = true;
        notify();
    }

private:

    bool _called;
};

class AMIRegular : public Test::AMI_Retry_op, public CallbackBase
{
public:

    virtual void ice_response()
    {
        called();
    }

    virtual void ice_exception(const ::Ice::Exception&)
    {
        test(false);
    }
};

typedef IceUtil::Handle<AMIRegular> AMIRegularPtr;

class AMIException : public Test::AMI_Retry_op, public CallbackBase
{
public:

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        test(dynamic_cast<const Ice::ConnectionLostException*>(&ex));
        called();
    }
};

typedef IceUtil::Handle<AMIException> AMIExceptionPtr;
#endif

RetryPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault(
        "Retry.Proxy", "retry:default -p 12010 -t 10000");
    Ice::ObjectPrx base1 = communicator->stringToProxy(ref);
    test(base1);
    Ice::ObjectPrx base2 = communicator->stringToProxy(ref);
    test(base2);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    RetryPrx retry1 = RetryPrx::checkedCast(base1);
    test(retry1);
    test(retry1 == base1);
    RetryPrx retry2 = RetryPrx::checkedCast(base2);
    test(retry2);
    test(retry2 == base2);
    tprintf("ok\n");

    tprintf("calling regular operation with first proxy... ");
    retry1->op(false);
    tprintf("ok\n");

    tprintf("calling operation to kill connection with second proxy... ");
    try
    {
        retry2->op(true);
        test(false);
    }
    catch(Ice::ConnectionLostException)
    {
        tprintf("ok\n");
    }

    tprintf("calling regular operation with first proxy again... ");
    retry1->op(false);
    tprintf("ok\n");

#ifdef ICEE_HAS_AMI
    AMIRegularPtr cb1 = new AMIRegular;
    AMIExceptionPtr cb2 = new AMIException;

    tprintf("calling regular AMI operation with first proxy... "); fflush(stdout);
    retry1->op_async(cb1, false);
    test(cb1->check());
    tprintf("ok\n");

    tprintf("calling AMI operation to kill connection with second proxy... "); fflush(stdout);
    retry2->op_async(cb2, true);
    test(cb2->check());
    tprintf("ok\n");

    tprintf("calling regular AMI operation with first proxy again... "); fflush(stdout);
    retry1->op_async(cb1, false);
    test(cb1->check());
    tprintf("ok\n");
#endif
    return retry1;
}
