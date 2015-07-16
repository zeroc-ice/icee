// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Initialize.h>
#include <Ice/RegisterPlugins.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

extern "C"
{

Plugin* createIceSSL(const CommunicatorPtr&, const string&, const StringSeq&);
Plugin* createCryptPermissionsVerifier(const CommunicatorPtr&, const string&, const StringSeq&);
Plugin* createIceDiscovery(const CommunicatorPtr&, const string&, const StringSeq&);
Plugin* createIceLocatorDiscovery(const CommunicatorPtr&, const string&, const StringSeq&);

}
namespace
{

class RegisterPluginsInit
{
public:

    RegisterPluginsInit()
    {
        Ice::registerPluginFactory("IceSSL", createIceSSL, true);
        Ice::registerPluginFactory("Glacier2CryptPermissionsVerifier", createCryptPermissionsVerifier, false);
        Ice::registerPluginFactory("IceDiscovery", createIceDiscovery, false);
        Ice::registerPluginFactory("IceLocatorDiscovery", createIceLocatorDiscovery, false);
    }
};


RegisterPluginsInit init;
}
