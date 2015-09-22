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
#include <TestApplication.h>
#include <TestI.h>

using namespace std;

class BindingTestApplication : public TestApplication
{
public:

    BindingTestApplication() :
        TestApplication("binding server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");

        loadConfig(initData.properties);

        //
        // This test requires a server thread pool with more than one thread.
        //
        initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "3");
        initData.properties->setProperty("Ice.ThreadPool.Server.SizeWarn", "0");

        initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::Identity id = communicator()->stringToIdentity("communicator");
        adapter->add(new RemoteCommunicatorI(), id);
        adapter->activate();

        // Disable ready print for further adapters.
        communicator()->getProperties()->setProperty("Ice.PrintAdapterReady", "0");

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
    BindingTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    BindingTestApplication app;
    return app.main(argc, argv);
}

#endif
