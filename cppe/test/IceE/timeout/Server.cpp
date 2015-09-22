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
#include <TestI.h>

using namespace std;

class TimeoutTestApplication : public TestApplication
{
public:

    TimeoutTestApplication() :
        TestApplication("timeout server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();

        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
        //initData.properties->setProperty("Ice.Trace.Network", "5");
        //initData.properties->setProperty("Ice.Trace.Protocol", "5");

        loadConfig(initData.properties);

        //
        // Now parse argc/argv into initData.properties
        //
        initData.properties = Ice::createProperties(argc, argv, initData.properties);

        //
        // We don't want connection warnings because of the timeout test.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");

        initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new TimeoutI;
        adapter->add(object, communicator()->stringToIdentity("timeout"));
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
    TimeoutTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    TimeoutTestApplication app;
    return app.main(argc, argv);
}

#endif
