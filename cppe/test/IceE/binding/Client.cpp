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
#include <Test.h>

using namespace std;

class BindingTestApplication : public TestApplication
{
public:

    BindingTestApplication() :
        TestApplication("binding client")
    {
    }

    virtual int
    run(int argc, char* argv[])
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

        void allTests(const Ice::CommunicatorPtr&);
        allTests(communicator());
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
