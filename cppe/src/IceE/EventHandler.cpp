// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/EventHandler.h>
#include <IceE/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(EventHandler* p) { return p; }
ICE_DECLSPEC_EXPORT IceUtil::Shared* IceInternal::upCast(ThreadPoolWorkItem* p) { return p; }

InstancePtr
IceInternal::EventHandler::instance() const
{
    return _instance;
}

IceInternal::EventHandler::EventHandler(const InstancePtr& instance, SOCKET fd) :
    _instance(instance),
    _stream(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
            , _instance->initializationData().stringConverter,
            _instance->initializationData().wstringConverter
#endif
           ),
    _fd(fd),
    _serializing(false),
    _registered(false)
{
}

IceInternal::EventHandler::~EventHandler()
{
}
