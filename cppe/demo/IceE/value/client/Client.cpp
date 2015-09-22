// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <ValueI.h>

using namespace std;
using namespace Demo;

class ObjectFactory : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr
    create(const std::string& type)
    {
        if(type == "::Demo::Printer")
        {
            return new PrinterI;
        }

        if(type == "::Demo::DerivedPrinter")
        {
            return new DerivedPrinterI;
        }

        assert(false);
        return 0;
    }

    virtual void destroy()
    {
        // Nothing to do
    }
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    if(argc > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return EXIT_FAILURE;
    }

    InitialPrx initial = InitialPrx::checkedCast(communicator->propertyToProxy("Initial.Proxy"));
    if(!initial)
    {
        fprintf(stderr, "%s: invalid object reference\n", argv[0]);
        return EXIT_FAILURE;
    }

    char c = EOF;

    printf("\n");
    printf("Let's first transfer a simple object, for a class without\n");
    printf("operations, and print its contents. No factory is required\n");
    printf("for this.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    SimplePtr simple = initial->getSimple();
    printf("==> %s\n", simple->message.c_str());

    printf("\n");
    printf("Yes, this worked. Now let's try to transfer an object for a class\n");
    printf("with operations as type ::Demo::Printer, without installing a factory first.\n");
    printf("This should give us a `no factory' exception.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    PrinterPtr printer;
    PrinterPrx printerProxy;
    try
    {
        initial->getPrinter(printer, printerProxy);
        fprintf(stderr, "%s: Did not get the expected NoObjectFactoryException!\n", argv[0]);
        return EXIT_FAILURE;
    }
    catch(const Ice::MarshalException& ex)
    {
        printf("==> %s\n", ex.toString().c_str());
    }

    printf("\n");
    printf("Yep, that's what we expected. Now let's try again, but with\n");
    printf("installing an appropriate factory first. If successful, we print\n");
    printf("the object's content.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    Ice::ObjectFactoryPtr factory = new ObjectFactory;
    communicator->addObjectFactory(factory, "::Demo::Printer");

    initial->getPrinter(printer, printerProxy);
    printf("==> %s\n", printer->message.c_str());

    printf("\n");
    printf("Cool, it worked! Let's try calling the printBackwards() method\n");
    printf("on the object we just received locally.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    printf("==> ");
    printer->printBackwards();

    printf("\n");
    printf("Now we call the same method, but on the remote object. Watch the\n");
    printf("server's output.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    printerProxy->printBackwards();

    printf("\n");
    printf("Next, we transfer a derived object from the server as a base\n");
    printf("object. Since we haven't yet installed a factory for the derived\n");
    printf("class, the derived class (::Demo::DerivedPrinter) is sliced\n");
    printf("to its base class (::Demo::Printer).\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    PrinterPtr derivedAsBase;
    derivedAsBase = initial->getDerivedPrinter();
    printf("==> The type ID of the received object is \"%s\"\n",  derivedAsBase->ice_id().c_str());
    assert(derivedAsBase->ice_id() == "::Demo::Printer");
    
    printf("\n");
    printf("Now we install a factory for the derived class, and try again.\n");
    printf("Because we receive the derived object as a base object, we\n");
    printf("we need to do a dynamic_cast<> to get from the base to the derived object.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');
    
    communicator->addObjectFactory(factory, "::Demo::DerivedPrinter");
    
    derivedAsBase = initial->getDerivedPrinter();
    DerivedPrinterPtr derived = DerivedPrinterPtr::dynamicCast(derivedAsBase);
    assert(derived);
    printf("==> dynamic_cast<> to derived object succeded\n");
    printf("==> The type ID of the received object is \"%s\"\n", derived->ice_id().c_str());

    printf("\n");
    printf("Let's print the message contained in the derived object, and\n");
    printf("call the operation printUppercase() on the derived object\n");
    printf("locally.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    printf("==> %s\n", derived->derivedMessage.c_str());
    printf("==> ");
    derived->printUppercase();

    printf("\n");
    printf("Finally, we try the same again, but instead of returning the\n");
    printf("derived object, we throw an exception containing the derived\n");
    printf("object.\n");
    printf("[press enter]\n"); fflush(stdout);
    do
    {
        c = getchar();
    }
    while(c != EOF && c != '\n');

    try
    {
        initial->throwDerivedPrinter();
        fprintf(stderr, "%s: Did not get the expected DerivedPrinterException!", argv[0]);
        exit(EXIT_FAILURE);
    }
    catch(const DerivedPrinterException& ex)
    {
        derived = ex.derived;
        if(!derived)
        {
            fprintf(stderr, "%s: Unexpected null pointer for `derived'", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("==> %s\n", derived->derivedMessage.c_str());
    printf("==> ");
    derived->printUppercase();

    printf("\nThat's it for this demo. Have fun with Ice!\n");

    initial->shutdown();

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

