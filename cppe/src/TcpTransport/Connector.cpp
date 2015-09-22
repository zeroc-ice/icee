// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Connector.h>
#include <IceE/Transceiver.h>
#include <IceE/Instance.h>
#include <IceE/TraceLevels.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Network.h>
#include <IceE/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Connector* p) { return p; }

TransceiverPtr
Connector::connect()
{
    if(_traceLevels->network >= 2)
    {
        Trace out(_logger, _traceLevels->networkCat);
        out << "trying to establish tcp connection to " << toString();
    }

    try
    {
        SOCKET fd = createSocket();
        setBlock(fd, false);
        setTcpBufSize(fd, _instance->initializationData().properties, _logger);
        bool connected = doConnect(fd, _addr);
        if(connected)
        {
            if(_traceLevels->network >= 1)
            {
                Trace out(_logger, _traceLevels->networkCat);
                out << "tcp connection established\n" << fdToString(fd);
            }
        }
        return new Transceiver(_instance, fd, connected);
    }
    catch(const Ice::LocalException& ex)
    {
        if(_traceLevels->network >= 2)
        {
            Trace out(_logger, _traceLevels->networkCat);
            out << "failed to establish tcp connection to " << toString() << "\n" << ex.toString();
        }
        throw;
    }
}

string
Connector::toString() const
{
    return addrToString(_addr);
}

bool
Connector::operator==(const Connector& r) const
{
    if(_timeout != r._timeout)
    {
        return false;
    }
    
    return compareAddress(_addr, r._addr) == 0;
}

bool
Connector::operator<(const Connector& r) const
{
    if(_timeout < r._timeout)
    {
        return true;
    }
    else if(r._timeout < _timeout)
    {
        return false;
    }

    return compareAddress(_addr, r._addr) == -1;
}


Connector::Connector(const InstancePtr& instance, const struct sockaddr_in& addr, int timeout) :
    _instance(instance),
    _addr(addr),
    _timeout(timeout),
    _traceLevels(instance->traceLevels()),
    _logger(instance->initializationData().logger)
{
}

Connector::~Connector()
{
}
