// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Acceptor.h>
#include <IceE/Transceiver.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Network.h>
#include <IceE/Exception.h>
#include <IceE/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Acceptor* p) { return p; }

SOCKET
IceInternal::Acceptor::fd()
{
    return _fd;
}

void
IceInternal::Acceptor::close()
{
    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "stopping to accept tcp connections at " << toString();
    }

    SOCKET fd = _fd;
    _fd = INVALID_SOCKET;
    closeSocket(fd);
}

void
IceInternal::Acceptor::listen()
{
    try
    {
        doListen(_fd, _backlog);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }

    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "accepting tcp connections at " << toString();
    }
}

TransceiverPtr
IceInternal::Acceptor::accept()
{
    SOCKET fd = doAccept(_fd);
    setBlock(fd, false);
    setTcpBufSize(fd, _instance->initializationData().properties, _logger);

    if(_traceLevels->network >= 1)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "accepted tcp connection\n" << fdToString(fd);
    }

    return new Transceiver(_instance, fd, true);
}

string
IceInternal::Acceptor::toString() const
{
    return addrToString(_addr);
}

int
IceInternal::Acceptor::effectivePort()
{
    return ntohs(_addr.sin_port);
}

IceInternal::Acceptor::Acceptor(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger)
{
#ifdef SOMAXCONN
    _backlog = instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.TCP.Backlog", SOMAXCONN);
#else 
    _backlog = instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.TCP.Backlog", 511);
#endif

    try
    {
        _fd = createSocket();
        _addr = getAddresses(host, port, true, true)[0];
        setTcpBufSize(_fd, _instance->initializationData().properties, _logger);
#ifndef _WIN32
        //
        // Enable SO_REUSEADDR on Unix platforms to allow re-using the
        // socket even if it's in the TIME_WAIT state. On Windows,
        // this doesn't appear to be necessary and enabling
        // SO_REUSEADDR would actually not be a good thing since it
        // allows a second process to bind to an address even it's
        // already bound by another process.
        //
        // TODO: using SO_EXCLUSIVEADDRUSE on Windows would probably
        // be better but it's only supported by recent Windows
        // versions (XP SP2, Windows Server 2003).
        //
        setReuseAddress(_fd, true);
#endif
        if(_traceLevels->network >= 2)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "attempting to bind to tcp socket " << toString();
        }
        doBind(_fd, _addr);
    }
    catch(...)
    {
        _fd = INVALID_SOCKET;
        throw;
    }
}

IceInternal::Acceptor::~Acceptor()
{
    assert(_fd == INVALID_SOCKET);
}
