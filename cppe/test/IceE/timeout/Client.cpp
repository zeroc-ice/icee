// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <TestApplication.h>
#include <Test.h>

using namespace std;
using namespace Test;

class TimeoutTestApplication : public TestApplication
{
public:

    TimeoutTestApplication() :
        TestApplication("timeout client")
    {
    }

    int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();

        loadConfig(initData.properties);

        //
        // Now parse argc/argv into initData.properties
        //
        initData.properties = Ice::createProperties(argc, argv, initData.properties);

        //
        // For this test, we want to disable retries.
        //
        initData.properties->setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");

        initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        TimeoutPrx allTests(const Ice::CommunicatorPtr&);
        TimeoutPrx timeout = allTests(communicator());
        timeout->shutdown();

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

