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
#include <set>

#include <functional>

using namespace std;
using namespace Test;

vector<string>
splitString(const string& str, const string& delim)
{
    string::size_type pos = 0;
    string::size_type length = str.length();
    string elt;
    vector<string> result;
    while(pos < length)
    {
        if(delim.find(str[pos]) != string::npos)
        {
            ++pos;
            if(elt.length() > 0)
            {
                result.push_back(elt);
                elt = "";
            }
        }
        else if(pos < length)
        {
            elt += str[pos++];
        }
    }
    if(elt.length() > 0)
    {
        result.push_back(elt);
    }
    return result;
}

#ifdef ICEE_HAS_AMI
class GetAdapterNameCB : public AMI_TestIntf_getAdapterName, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void
    ice_response(const string& name)
    {
        Lock sync(*this);
        assert(!name.empty());
        _name = name;
        notify();
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        test(false);
    }

    virtual string
    getResult()
    {
        Lock sync(*this);
        while(_name.empty())
        {
            wait();
        }
        return _name;
    }

private:

    string _name;
};
typedef IceUtil::Handle<GetAdapterNameCB> GetAdapterNameCBPtr;

string
getAdapterNameWithAMI(const TestIntfPrx& test)
{
    GetAdapterNameCBPtr cb = new GetAdapterNameCB();
    test->getAdapterName_async(cb);
    return cb->getResult();
}
#endif

TestIntfPrx
createTestIntfPrx(vector<RemoteObjectAdapterPrx>& adapters)
{
    vector<string> endpoints;
    TestIntfPrx test;
    for(vector<RemoteObjectAdapterPrx>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        test = (*p)->getTestIntf();
        vector<string> endpts = splitString(test->ice_toString(), ":");
        assert(endpts.size() > 1);
        endpoints.insert(endpoints.end(), endpts.begin() + 1, endpts.end());
    }
    string proxy = test->ice_getCommunicator()->identityToString(test->ice_getIdentity());
    for(vector<string>::const_iterator q = endpoints.begin(); q != endpoints.end(); ++q)
    {
        proxy += ":" + *q;
    }
    return TestIntfPrx::uncheckedCast(test->ice_getCommunicator()->stringToProxy(proxy));
}

void
deactivate(const RemoteCommunicatorPrx& com, vector<RemoteObjectAdapterPrx>& adapters)
{
    for(vector<RemoteObjectAdapterPrx>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        com->deactivateObjectAdapter(*p);
    }
}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "communicator:default -p 12010 -t 10000";
    RemoteCommunicatorPrx com = RemoteCommunicatorPrx::uncheckedCast(communicator->stringToProxy(ref));

    tprintf("testing binding with single endpoint... ");
    {
        RemoteObjectAdapterPrx adapter = com->createObjectAdapter("Adapter", "default");

        TestIntfPrx test1 = adapter->getTestIntf();
        TestIntfPrx test2 = adapter->getTestIntf();
        test(test1->ice_getConnection() == test2->ice_getConnection());

        test1->ice_ping();
        test2->ice_ping();
        
        com->deactivateObjectAdapter(adapter);
        
        TestIntfPrx test3 = TestIntfPrx::uncheckedCast(test1);
        test(test3->ice_getConnection() == test1->ice_getConnection());
        test(test3->ice_getConnection() == test2->ice_getConnection());
        
        try
        {
            test3->ice_ping();
            test(false);
        }
        catch(const Ice::ConnectionRefusedException&)
        {
        }
    }
    tprintf("ok\n");


    tprintf("testing binding with multiple endpoints... ");
    {
        vector<RemoteObjectAdapterPrx> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter11", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter12", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter13", "default"));

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        set<string> names;
        names.insert("Adapter11");
        names.insert("Adapter12");
        names.insert("Adapter13");
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrx> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());
            
            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close(false);
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for(vector<RemoteObjectAdapterPrx>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
            {
                (*p)->getTestIntf()->ice_ping();
            }
            
            TestIntfPrx test = createTestIntfPrx(adapters);
            string name = test->getAdapterName();
            const int nRetry = 10;
            int i;
            for(i = 0; i < nRetry &&  test->getAdapterName() == name; i++);
            test(i == nRetry);

            for(vector<RemoteObjectAdapterPrx>::const_iterator q = adapters.begin(); q != adapters.end(); ++q)
            {
                (*q)->getTestIntf()->ice_getConnection()->close(false);
            }
        }           

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        com->deactivateObjectAdapter(adapters[0]);
        names.insert("Adapter12");
        names.insert("Adapter13");
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrx> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);
            
            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close(false);
        }
        
        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        com->deactivateObjectAdapter(adapters[2]);      
        TestIntfPrx test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "Adapter12");    

        deactivate(com, adapters);
    }
    tprintf("ok\n");

#ifdef ICEE_HAS_AMI
    tprintf("testing binding with multiple endpoints and AMI... ");
    {
        vector<RemoteObjectAdapterPrx> adapters;
        adapters.push_back(com->createObjectAdapter("AdapterAMI11", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI12", "default"));
        adapters.push_back(com->createObjectAdapter("AdapterAMI13", "default"));

        //
        // Ensure that when a connection is opened it's reused for new
        // proxies and that all endpoints are eventually tried.
        //
        set<string> names;
        names.insert("AdapterAMI11");
        names.insert("AdapterAMI12");
        names.insert("AdapterAMI13");
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrx> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);

            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());
            
            names.erase(getAdapterNameWithAMI(test1));
            test1->ice_getConnection()->close(false);
        }

        //
        // Ensure that the proxy correctly caches the connection (we
        // always send the request over the same connection.)
        //
        {
            for(vector<RemoteObjectAdapterPrx>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
            {
                (*p)->getTestIntf()->ice_ping();
            }
            
            TestIntfPrx test = createTestIntfPrx(adapters);
            string name = getAdapterNameWithAMI(test);
            const int nRetry = 10;
            int i;
            for(i = 0; i < nRetry && getAdapterNameWithAMI(test) == name; i++);
            test(i == nRetry);

            for(vector<RemoteObjectAdapterPrx>::const_iterator q = adapters.begin(); q != adapters.end(); ++q)
            {
                (*q)->getTestIntf()->ice_getConnection()->close(false);
            }
        }           

        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapters.
        //
        com->deactivateObjectAdapter(adapters[0]);
        names.insert("AdapterAMI12");
        names.insert("AdapterAMI13");
        while(!names.empty())
        {
            vector<RemoteObjectAdapterPrx> adpts = adapters;

            TestIntfPrx test1 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test2 = createTestIntfPrx(adpts);
            random_shuffle(adpts.begin(), adpts.end());
            TestIntfPrx test3 = createTestIntfPrx(adpts);
            
            test(test1->ice_getConnection() == test2->ice_getConnection());
            test(test2->ice_getConnection() == test3->ice_getConnection());

            names.erase(test1->getAdapterName());
            test1->ice_getConnection()->close(false);
        }
        
        //
        // Deactivate an adapter and ensure that we can still
        // establish the connection to the remaining adapter.
        //
        com->deactivateObjectAdapter(adapters[2]);      
        TestIntfPrx test = createTestIntfPrx(adapters);
        test(test->getAdapterName() == "AdapterAMI12"); 

        deactivate(com, adapters);
    }
    tprintf("ok\n");
#endif
    
    tprintf("testing random endpoint selection... ");
    {
        vector<RemoteObjectAdapterPrx> adapters;
        adapters.push_back(com->createObjectAdapter("Adapter21", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter22", "default"));
        adapters.push_back(com->createObjectAdapter("Adapter23", "default"));

        TestIntfPrx test = createTestIntfPrx(adapters);
        //test(test->ice_getEndpointSelection() == Ice::Random);

        set<string> names;
        names.insert("Adapter21");
        names.insert("Adapter22");
        names.insert("Adapter23");
        while(!names.empty())
        {
            names.erase(test->getAdapterName());
            test->ice_getConnection()->close(false);
        }

        //test = TestIntfPrx::uncheckedCast(test->ice_endpointSelection(Ice::Random));
        //test(test->ice_getEndpointSelection() == Ice::Random);

        names.insert("Adapter21");
        names.insert("Adapter22");
        names.insert("Adapter23");
        while(!names.empty())
        {
            names.erase(test->getAdapterName());
            test->ice_getConnection()->close(false);
        }

        deactivate(com, adapters);
    }
    tprintf("ok\n");

    com->shutdown();
}
