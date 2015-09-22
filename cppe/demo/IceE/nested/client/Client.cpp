// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <NestedI.h>

using namespace std;
using namespace Demo;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    if(argc > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return EXIT_FAILURE;
    }

    NestedPrx nested = NestedPrx::checkedCast(communicator->propertyToProxy("Nested.Proxy"));
    if(!nested)
    {
        fprintf(stderr, "%s: invalid proxy\n", argv[0]);
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Nested.Client");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(communicator->stringToIdentity("nestedClient")));
    NestedPtr servant = new NestedI(self);
    adapter->add(servant, communicator->stringToIdentity("nestedClient"));
    adapter->activate();

    printf("Note: The maximum nesting level is sz * 2, with sz being\n");
    printf("the maximum number of threads in the server thread pool. if\n");
    printf("you specify a value higher than that, the application will\n");
    printf("block or timeout.\n\n");


    char buf[64];
    unsigned int size = 64;
    do
    {
        try
        {
            printf("enter nesting level or 'x' for exit: "); fflush(stdout);
            if(fgets(buf, size, stdin) == NULL)
            {
                break;
            }
            int level = atoi(buf);
            if(level > 0)
            {
                nested->nestedCall(level, self);
            }
        }
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str()); fflush(stderr);
        }
    }
    while(buf[0] != 'x');

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->load("config");
        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        fprintf(stderr, "%s\n", ex.toString().c_str());
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;
}
