// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Initialize.h>
#include <Ice/RegisterPluginsInit.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

extern "C"
{

Plugin* createIceUDP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);

}

RegisterPluginsInit::RegisterPluginsInit()
{
    Ice::registerPluginFactory("IceUDP", createIceUDP, true);
    Ice::registerPluginFactory("IceTCP", createIceTCP, true);
}
