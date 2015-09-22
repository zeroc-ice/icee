// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
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
            if(!timedWait(IceUtil::Time::seconds(30)))
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

class AMI_Test_pidI : virtual public AMI_TestIntf_pid, virtual public CallbackBase
{
public:

    virtual void ice_response(Ice::Int pid)
    {
        _pid = pid;
        called();
    }

    virtual void ice_exception(const Ice::Exception& ex)
    {
        test(false);
    }

    Ice::Int pid() const
    {
        return _pid;
    }

private:

    Ice::Int _pid;
};

typedef IceUtil::Handle<AMI_Test_pidI> AMI_Test_pidIPtr;

class AMI_Test_shutdownI : virtual public AMI_TestIntf_shutdown, virtual public CallbackBase
{
public:

    virtual void ice_response()
    {
        called();
    }

    virtual void ice_exception(const Ice::Exception&)
    {
        test(false);
    }
};

typedef IceUtil::Handle<AMI_Test_shutdownI> AMI_Test_shutdownIPtr;

class AMI_Test_abortI : virtual public AMI_TestIntf_abort, virtual public CallbackBase
{
public:

    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
        catch(Ice::Exception& ex)
        {
            tprintf("%s\n", ex.what());
            test(false);
        }
        called();
    }
};

typedef IceUtil::Handle<AMI_Test_abortI> AMI_Test_abortIPtr;

class AMI_Test_idempotentAbortI : public AMI_TestIntf_idempotentAbort, public CallbackBase
{
    virtual void ice_response()
    {
        test(false);
    }

    virtual void ice_exception(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        catch(const Ice::ConnectFailedException&)
        {
        }
        catch(Ice::Exception& ex)
        {
            tprintf("%s\n", ex.what());
            test(false);
        }
        called();
    }
};

typedef IceUtil::Handle<AMI_Test_idempotentAbortI> AMI_Test_idempotentAbortIPtr;

#endif

void
allTests(const Ice::CommunicatorPtr& communicator, const vector<int>& ports)
{
    tprintf("testing stringToProxy...");
    string ref("test");

    char buf[32];
    for(vector<int>::const_iterator p = ports.begin(); p != ports.end(); ++p)
    {
        sprintf(buf, ":default -t 60000 -p %d", *p);
        ref += buf;
    }
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast...");
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    tprintf("ok\n");

    int oldPid = 0;
    bool ami = false;
    for(unsigned int i = 1, j = 0; i <= ports.size(); ++i, ++j)
    {
        if(j > 3)
        {
            j = 0;
            ami = !ami;
        }

#ifdef ICEE_HAS_AMI
        if(!ami)
#endif
        {
            tprintf("testing server #%d...", i);
            int pid = obj->pid();
            test(pid != oldPid);
            tprintf("ok\n");
            oldPid = pid;
        }
#ifdef ICEE_HAS_AMI
        else
        {
            tprintf("testing server #%d with AMI... ", i);
            AMI_Test_pidIPtr cb = new AMI_Test_pidI();
            obj->pid_async(cb);
            test(cb->check());
            int pid = cb->pid();
            test(pid != oldPid);
            tprintf("ok\n");
            oldPid = pid;
        }
#endif
            
        if(j == 0)
        {
#ifdef ICEE_HAS_AMI
            if(!ami)
#endif
            {
                tprintf("shutting down server #%d...", i);
                obj->shutdown();
                tprintf("ok\n");
            }
#ifdef ICEE_HAS_AMI
            else
            {
                tprintf("shutting down server #%d with AMI...", i);
                AMI_Test_shutdownIPtr cb = new AMI_Test_shutdownI;
                obj->shutdown_async(cb);
                test(cb->check());
                tprintf("ok\n");
            }
#endif
        }
        else if(j == 1 || i + 1 > ports.size())
        {
#ifdef ICEE_HAS_AMI
            if(!ami)
#endif
            {
                tprintf("aborting server #%d...", i);
                try
                {
                    obj->abort();
                    test(false);
                }
                catch(const Ice::ConnectionLostException&)
                {
                    tprintf("ok\n");
                }
                catch(const Ice::ConnectFailedException&)
                {
                    tprintf("ok\n");
                }
            }
#ifdef ICEE_HAS_AMI
            else
            {
                tprintf("aborting server #%d with AMI...", i);
                AMI_Test_abortIPtr cb = new AMI_Test_abortI;
                obj->abort_async(cb);
                test(cb->check());
                tprintf("ok\n");
            }
#endif
        }
        else if(j == 2 || j == 3)
        {
#ifdef ICEE_HAS_AMI
            if(!ami)
#endif
            {
                tprintf("aborting server #%d and #%d with idempotent call...", i, i + 1);
                try
                {
                    obj->idempotentAbort();
                    test(false);
                }
                catch(const Ice::ConnectionLostException&)
                {
                    tprintf("ok\n");
                }
                catch(const Ice::ConnectFailedException&)
                {
                    tprintf("ok\n");
                }
            }
#ifdef ICEE_HAS_AMI
            else
            {
                tprintf("aborting server #%d and #%d with idempotent AMI call...", i, i + 1);
                AMI_Test_idempotentAbortIPtr cb = new AMI_Test_idempotentAbortI;
                obj->idempotentAbort_async(cb);
                test(cb->check());
                tprintf("ok\n");
            }
#endif
            ++i;
        }
        else
        {
            assert(false);
        }
    }

    tprintf("testing whether all servers are gone...");
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch(const Ice::LocalException&)
    {
        tprintf("ok\n");
    }
}
