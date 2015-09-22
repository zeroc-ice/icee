// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class AMI_Hello_sayHelloI : public AMI_Hello_sayHello
{
public:

    virtual void ice_response()
    {
    }

    virtual void ice_exception(const Ice::Exception& ex)
    {
        fprintf(stderr, "sayHello AMI called failed:\n%s\n", ex.toString().c_str());
    }
};

void
menu()
{
    printf("usage:\n");
    printf("i: send immediate greeting\n");
    printf("d: send delayed greeting\n");
    printf("s: shutdown server\n");
    printf("x: exit\n");
    printf("?: help\n");
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    if(argc > 1)
    {
        fprintf(stderr, "%s: too many arguments", argv[0]);
        return EXIT_FAILURE;
    }

    HelloPrx hello = HelloPrx::checkedCast(communicator->propertyToProxy("Hello.Proxy"));
    if(!hello)
    {
        fprintf(stderr, "%s: invalid proxy\n", argv[0]);
        return EXIT_FAILURE;
    }

    menu();

    char c = EOF;
    do
    {
        try
        {
            printf("==> "); fflush(stdout);
            do
            {
                c = getchar();
            }
            while(c != EOF && c == '\n');
            if(c == 'i')
            {
                hello->sayHello(0);
            }
            else if(c == 'd')
            {
                hello->sayHello_async(new AMI_Hello_sayHelloI, 5000);
            }
            else if(c == 's')
            {
                hello->shutdown();
            }
            else if(c == 'x')
            {
                // Nothing to do
            }
            else if(c == '?')
            {
                menu();
            }
            else
            {
                printf("unknown command `%c'\n", c);
                menu();
            }
        }
        catch(const Ice::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
        }
    }
    while(c != EOF && c != 'x');

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
