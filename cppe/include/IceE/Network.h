// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_NETWORK_H
#define ICEE_NETWORK_H

#ifdef __hpux
#   define _XOPEN_SOURCE_EXTENDED
#endif

#include <IceE/Config.h>
#include <IceE/PropertiesF.h> // For setTcpBufSize
#include <IceE/LoggerF.h> // For setTcpBufSize

#ifdef _WIN32
#   include <winsock2.h>
typedef int ssize_t;
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/socket.h>

#   if defined(__hpux)
#      include <sys/time.h>
#   else   
#      include <sys/poll.h>
#   endif

#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#endif

#ifdef _WIN32
typedef int socklen_t;
#endif

#ifndef _WIN32
#   define SOCKET int
#   define SOCKET_ERROR -1
#   define INVALID_SOCKET -1
#endif

#ifndef SHUT_RD
#   define SHUT_RD 0
#endif

#ifndef SHUT_WR
#   define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
#   define SHUT_RDWR 2
#endif

#ifndef NETDB_INTERNAL
#   define NETDB_INTERNAL -1
#endif

#ifndef NETDB_SUCCESS
#   define NETDB_SUCCESS 0
#endif

namespace IceInternal
{

bool interrupted();
bool acceptInterrupted();
bool noBuffers();
bool wouldBlock();
bool timedout();
bool connectFailed();
bool connectionRefused();
bool connectInProgress();
bool connectionLost();
bool notConnected();
bool noMoreFds(int);

SOCKET createSocket();
void closeSocket(SOCKET);

void setBlock(SOCKET, bool);
#ifndef ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
void setTimeout(SOCKET, bool, int);
#endif
void setTcpNoDelay(SOCKET);
void setKeepAlive(SOCKET);
int getSendBufferSize(SOCKET);
void setSendBufferSize(SOCKET, int);
#ifndef _WIN32_WCE
void setRecvBufferSize(SOCKET, int);
int getRecvBufferSize(SOCKET);
#endif
void setReuseAddress(SOCKET, bool);

void doBind(SOCKET, struct sockaddr_in&);
void doListen(SOCKET, int);
bool doConnect(SOCKET, struct sockaddr_in&);
void doFinishConnect(SOCKET);
SOCKET doAccept(SOCKET);

std::vector<struct sockaddr_in> getAddresses(const std::string&, int, bool, bool);
int compareAddress(const struct sockaddr_in&, const struct sockaddr_in&);

void createPipe(SOCKET fds[2]);

std::string errorToStringDNS(int);

std::string fdToString(SOCKET);
void fdToLocalAddress(SOCKET, struct sockaddr_in&);
bool fdToRemoteAddress(SOCKET, struct sockaddr_in&);
std::string addrToString(const struct sockaddr_in&);
std::vector<std::string> getHostsForEndpointExpand(const std::string&);

void setTcpBufSize(SOCKET, const Ice::PropertiesPtr&, const Ice::LoggerPtr&);

int getSocketErrno();

}

#endif
