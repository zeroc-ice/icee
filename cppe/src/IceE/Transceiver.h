// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRANSCEIVER_H
#define ICEE_TRANSCEIVER_H

#include <IceE/Shared.h>
#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>
#include <IceE/SelectorF.h>

#ifdef _WIN32
#   include <winsock2.h>
typedef int ssize_t;
#else
#   define SOCKET int
#endif

namespace IceInternal
{

class Connector;
class Acceptor;
class Buffer;

class Transceiver : public ::IceUtil::Shared
{
    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateConnected
    };

public:

    SOCKET fd();
    void close();
    bool write(Buffer&);
    bool read(Buffer&);
    std::string type() const;
    std::string toString() const;
    SocketStatus initialize();
    void checkSendSize(const Buffer&, size_t);

private:

    Transceiver(const InstancePtr&, SOCKET, bool);
    virtual ~Transceiver();
    friend class Connector;
    friend class Acceptor;

    const TraceLevelsPtr _traceLevels;
    const Ice::LoggerPtr _logger;

    SOCKET _fd;
    State _state;
    std::string _desc;
#ifdef _WIN32
    int _maxPacketSize;
#endif
};

}

#endif
