// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <Test.h>
#include <MyObjectI.h>
#include <InterceptorI.h>

using namespace std;

class InterceptorTestApplication : public TestApplication
{
public:

    InterceptorTestApplication() :
        TestApplication("interceptor client")
    {
    }

    virtual int run(int, char*[]);
    
private:

    int run(const Test::MyObjectPrx&, const InterceptorIPtr&);
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    InterceptorTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    InterceptorTestApplication app;
    return app.main(argc, argv);
}

#endif

int
InterceptorTestApplication::run(int argc, char* argv[])
{
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();

    loadConfig(initData.properties);

    initData.properties->setProperty("Ice.Warn.Dispatch", "0");

    initData.logger = getLogger();
    setCommunicator(Ice::initialize(argc, argv, initData));

    //
    // Create OA and servants  
    //  
    Ice::ObjectAdapterPtr oa = communicator()->createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost");
    
    Ice::ObjectPtr servant = new MyObjectI;
    InterceptorIPtr interceptor = new InterceptorI(servant);
    
    Test::MyObjectPrx prx = Test::MyObjectPrx::uncheckedCast(oa->addWithUUID(interceptor));
    
    oa->activate();
       
    tprintf("testing simple interceptor... ");
    test(interceptor->getLastOperation().empty());
    prx->ice_ping();
    test(interceptor->getLastOperation() == "ice_ping");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    string typeId = prx->ice_id();
    test(interceptor->getLastOperation() == "ice_id");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    test(prx->ice_isA(typeId));
    test(interceptor->getLastOperation() == "ice_isA");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    test(prx->add(33, 12) == 45);
    test(interceptor->getLastOperation() == "add");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    tprintf("ok\n");

    tprintf("testing retry... ");
    test(prx->addWithRetry(33, 12) == 45);
    test(interceptor->getLastOperation() == "addWithRetry");
    test(interceptor->getLastStatus() == Ice::DispatchOK);
    tprintf("ok\n");

    tprintf("testing user exception... ");
    try
    {
        prx->badAdd(33, 12);
        test(false);
    }
    catch(const Test::InvalidInputException&)
    {
        // expected
    }
    test(interceptor->getLastOperation() == "badAdd");
    test(interceptor->getLastStatus() == Ice::DispatchUserException);
    tprintf("ok\n");

    tprintf("testing ONE... ");
    interceptor->clear();
    try
    {
        prx->notExistAdd(33, 12);
        test(false);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        // expected
    }
    test(interceptor->getLastOperation() == "notExistAdd");
    tprintf("ok\n");

    tprintf("testing system exception... ");
    interceptor->clear();
    try
    {
        prx->badSystemAdd(33, 12);
        test(false);
    }
    catch(const Ice::UnknownLocalException&)
    {
    }
    catch(...)
    {
        test(false);
    }
    test(interceptor->getLastOperation() == "badSystemAdd");
    tprintf("ok\n");

    return 0;
}
