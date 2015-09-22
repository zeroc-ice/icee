// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <TestI.h>

using namespace std;
using namespace Test;

class MyObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const string& type)
    {
        if(type == "::Test::I")
        {
            return new II;
        }
        else if(type == "::Test::J")
        {
            return new JI;
        }
        else if(type == "::Test::H")
        {
            return new HI;
        }

        assert(false); // Should never be reached
        return 0;
    }

    virtual void destroy()
    {
        // Nothing to do
    }
};


class ObjectsTestApplication : public TestApplication
{
public:

    ObjectsTestApplication() :
        TestApplication("objects server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");

        loadConfig(initData.properties);
        initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectFactoryPtr factory = new MyObjectFactory;
        communicator()->addObjectFactory(factory, "::Test::I");
        communicator()->addObjectFactory(factory, "::Test::J");
        communicator()->addObjectFactory(factory, "::Test::H");

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        InitialPtr initial = new InitialI(adapter);
        adapter->add(initial, communicator()->stringToIdentity("initial"));
        adapter->activate();

#ifndef _WIN32_WCE
        communicator()->waitForShutdown();
#endif

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
