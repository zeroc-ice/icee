// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <TestI.h>
#include <MyObjectFactory.h>

using namespace std;
using namespace Test;

class ObjectsTestApplication : public TestApplication
{
public:

    ObjectsTestApplication() :
        TestApplication("objects client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        loadConfig(initData.properties);
        initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectFactoryPtr factory = new MyObjectFactory;
        communicator()->addObjectFactory(factory, "::Test::B");
        communicator()->addObjectFactory(factory, "::Test::C");
        communicator()->addObjectFactory(factory, "::Test::D");
        communicator()->addObjectFactory(factory, "::Test::E");
        communicator()->addObjectFactory(factory, "::Test::F");
        communicator()->addObjectFactory(factory, "::Test::I");
        communicator()->addObjectFactory(factory, "::Test::J");
        communicator()->addObjectFactory(factory, "::Test::H");

        InitialPrx allTests(const Ice::CommunicatorPtr&);
        InitialPrx initial = allTests(communicator());
        initial->shutdown();
        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    ObjectsTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    ObjectsTestApplication app;
    return app.main(argc, argv);
}

#endif
