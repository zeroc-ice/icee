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
#include <Test.h>
#include <TestApplication.h>
#include <iostream>

using namespace std;

class PriorityTestApplication : public TestApplication
{
public:

    PriorityTestApplication() :
        TestApplication("thread pool priority client")
    {
    }

    int
    run(int argc, char** argv)
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();

        loadConfig(initData.properties);

        //
        // Now parse argc/argv into initData
        //
        initData.properties = Ice::createProperties(argc, argv, initData.properties); 

        initData.logger = getLogger();        
        setCommunicator(Ice::initialize(argc, argv, initData));

        Test::PriorityPrx allTests(const Ice::CommunicatorPtr&);
        Test::PriorityPrx myClass = allTests(communicator());

        myClass->shutdown();

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    PriorityTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char* argv[])
{
    PriorityTestApplication app;
    return app.main(argc, argv);
}
#endif

