// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Network.h>
#include <IceE/LocalException.h>
#include <IceE/Properties.h> // For setTcpBufSize
#include <IceE/LoggerUtil.h> // For setTcpBufSize
#include <IceE/StringUtil.h>
#include <IceE/SafeStdio.h>

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#  include <ifaddrs.h>
#  include <net/if.h>
#else
#  include <sys/ioctl.h>
#  include <net/if.h>
#  ifdef __sun
#    include <sys/sockio.h>
#  endif
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifdef __sun
#    define INADDR_NONE (unsigned long)-1
#endif

namespace
{

void
closeSocketNoThrow(SOCKET fd)
{
#ifdef _WIN32
    int error = WSAGetLastError();
    closesocket(fd);
    WSASetLastError(error);
#else
    int error = errno;
    close(fd);
    errno = error;
#endif
}

string
inetAddrToString(const struct sockaddr_in& in)
{
    char namebuf[1024];
    namebuf[0] = '\0';
    getnameinfo(reinterpret_cast<const struct sockaddr *>(&in), sizeof(sockaddr_in), namebuf, sizeof(namebuf), 0, 0, 
                NI_NUMERICHOST);
    return string(namebuf);
}

vector<struct sockaddr_in>
getLocalAddresses()
{
    vector<struct sockaddr_in> result;

#if defined(_WIN32)
    try
    {
        SOCKET fd = createSocket();

        vector<unsigned char> buffer;
        buffer.resize(1024);
        unsigned long len = 0;
        DWORD rs = WSAIoctl(fd, SIO_ADDRESS_LIST_QUERY, 0, 0, 
                            &buffer[0], static_cast<DWORD>(buffer.size()),
                            &len, 0, 0);
        if(rs == SOCKET_ERROR)
        {
            //
            // If the buffer wasn't big enough, resize it to the
            // required length and try again.
            //
            if(getSocketErrno() == WSAEFAULT)
            {
                buffer.resize(len);
                rs = WSAIoctl(fd, SIO_ADDRESS_LIST_QUERY, 0, 0,
                              &buffer[0], static_cast<DWORD>(buffer.size()),
                              &len, 0, 0);
            }

            if(rs == SOCKET_ERROR)
            {
                closeSocketNoThrow(fd);
                SocketException ex(__FILE__, __LINE__);
                ex.error = getSocketErrno();
                throw ex;
            }
        }

        //
        // Add the local interface addresses.
        //
        SOCKET_ADDRESS_LIST* addrs = reinterpret_cast<SOCKET_ADDRESS_LIST*>(&buffer[0]);
        for (int i = 0; i < addrs->iAddressCount; ++i)
        {
            
            result.push_back(*reinterpret_cast<struct sockaddr_in*>(addrs->Address[i].lpSockaddr));
        }

        closeSocket(fd);
    }
    catch(const Ice::LocalException&)
    {
        //
        // TODO: Warning?
        //
    }
#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct ifaddrs* ifap;
    if(::getifaddrs(&ifap) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    struct ifaddrs* curr = ifap;
    while(curr != 0)
    {
        if(curr->ifa_addr && curr->ifa_addr->sa_family == AF_INET && !(curr->ifa_flags & IFF_LOOPBACK))
        {
            struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(curr->ifa_addr);
            if(addr->sin_addr.s_addr != 0)
            {
                result.push_back(*addr);
            }
        }

        curr = curr->ifa_next;
    }

    ::freeifaddrs(ifap);
#else
    SOCKET fd = createSocket();

#ifdef _AIX
    int cmd = CSIOCGIFCONF;
#else
    int cmd = SIOCGIFCONF;
#endif
    struct ifconf ifc;
    int numaddrs = 10;
    int old_ifc_len = 0;

    //
    // Need to call ioctl multiple times since we do not know up front
    // how many addresses there will be, and thus how large a buffer we need.
    // We keep increasing the buffer size until subsequent calls return
    // the same length, meaning we have all the addresses.
    //
    while(true)
    {
        int bufsize = numaddrs * sizeof(struct ifreq);
        ifc.ifc_len = bufsize;
        ifc.ifc_buf = (char*)malloc(bufsize);
        int rs = ioctl(fd, cmd, &ifc);
        if(rs == SOCKET_ERROR)
        {
            free(ifc.ifc_buf);
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(ifc.ifc_len == old_ifc_len)
        {
            //
            // Returned same length twice in a row, finished.
            //
            break;
        }
        else
        {
            old_ifc_len = ifc.ifc_len;
        }

        numaddrs += 10;
        free(ifc.ifc_buf);
    }

    numaddrs = ifc.ifc_len / sizeof(struct ifreq);
    struct ifreq* ifr = ifc.ifc_req;
    for(int i = 0; i < numaddrs; ++i)
    {
        if(ifr[i].ifr_addr.sa_family == AF_INET && !(ifr[i].ifr_flags & IFF_LOOPBACK))
        {
            struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&ifr[i].ifr_addr);
            if(addr->sin_addr.s_addr != 0)
            {
                result.push_back(*addr);
            }
        }
    }

    free(ifc.ifc_buf);
    closeSocket(fd);
#endif

    return result;
}

}

int
IceInternal::getSocketErrno()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool
IceInternal::interrupted()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEINTR;
#else
#   ifdef EPROTO
    return errno == EINTR || errno == EPROTO;
#   else
    return errno == EINTR;
#   endif
#endif
}

bool
IceInternal::noBuffers()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAENOBUFS ||
           error == WSAEFAULT;
#else
    return errno == ENOBUFS;
#endif
}

bool
IceInternal::wouldBlock()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEWOULDBLOCK;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

bool
IceInternal::timedout()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAETIMEDOUT;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

bool
IceInternal::connectFailed()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED ||
           error == WSAETIMEDOUT ||
           error == WSAENETUNREACH ||
           error == WSAEHOSTUNREACH ||
           error == WSAECONNRESET ||
           error == WSAESHUTDOWN ||
           error == WSAECONNABORTED;
#else
    return errno == ECONNREFUSED ||
           errno == ETIMEDOUT ||
           errno == ENETUNREACH ||
           errno == EHOSTUNREACH ||
           errno == ECONNRESET ||
           errno == ESHUTDOWN ||
           errno == ECONNABORTED;
#endif
}

bool
IceInternal::connectionRefused()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED;
#else
    return errno == ECONNREFUSED;
#endif
}

bool
IceInternal::connectInProgress()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEWOULDBLOCK;
#else
    return errno == EINPROGRESS;
#endif
}

bool
IceInternal::connectionLost()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNRESET ||
           error == WSAESHUTDOWN ||
           error == WSAENOTCONN ||
           error == WSAECONNABORTED;
#else
    return errno == ECONNRESET ||
           errno == ENOTCONN ||
           errno == ESHUTDOWN ||
           errno == ECONNABORTED ||
           errno == EPIPE;
#endif
}

bool
IceInternal::notConnected()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAENOTCONN;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    return errno == ENOTCONN || errno == EINVAL;
#else
    return errno == ENOTCONN;
#endif
}

bool
IceInternal::noMoreFds(int error)
{
#ifdef _WIN32
    return error == WSAEMFILE;
#else
    return error == EMFILE || error == ENFILE;
#endif
}

SOCKET
IceInternal::createSocket()
{
    SOCKET fd;

    fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd == INVALID_SOCKET)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    setTcpNoDelay(fd);
    setKeepAlive(fd);

    return fd;
}

void
IceInternal::closeSocket(SOCKET fd)
{
#ifdef _WIN32
    int error = WSAGetLastError();
    if(closesocket(fd) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    WSASetLastError(error);
#else
    int error = errno;
    if(close(fd) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    errno = error;
#endif
}
    
void
IceInternal::setBlock(SOCKET fd, bool block)
{
    if(block)
    {
#ifdef _WIN32
        unsigned long arg = 0;
        if(ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = WSAGetLastError();
            throw ex;
        }
#else
        int flags = fcntl(fd, F_GETFL);
        flags &= ~O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = errno;
            throw ex;
        }
#endif
    }
    else
    {
#ifdef _WIN32
        unsigned long arg = 1;
        if(ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = WSAGetLastError();
            throw ex;
        }
#else
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = errno;
            throw ex;
        }
#endif
    }
}

void
IceInternal::setTcpNoDelay(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}
    
void
IceInternal::setKeepAlive(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&flag, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

int
IceInternal::getSendBufferSize(SOCKET fd)
{
    int sz;
    socklen_t len = sizeof(sz);
    if(getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, &len) == SOCKET_ERROR || len != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    return sz;
}

void
IceInternal::setSendBufferSize(SOCKET fd, int sz)
{
    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

#ifndef _WIN32_WCE
void
IceInternal::setRecvBufferSize(SOCKET fd, int sz)
{
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

int
IceInternal::getRecvBufferSize(SOCKET fd)
{
    int sz;
    socklen_t len = sizeof(sz);
    if(getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, &len) == SOCKET_ERROR || len != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
    return sz;
}
#endif // _WIN32_WCE

void
IceInternal::setReuseAddress(SOCKET fd, bool reuse)
{
    int flag = reuse ? 1 : 0;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::doBind(SOCKET fd, struct sockaddr_in& addr)
{
    if(bind(fd, reinterpret_cast<struct sockaddr*>(&addr), int(sizeof(addr))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    socklen_t len = static_cast<socklen_t>(sizeof(addr));
#ifdef NDEBUG
    getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len);
#else
    int ret = getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len);
    assert(ret != SOCKET_ERROR);
#endif
}

bool
IceInternal::doConnect(SOCKET fd, struct sockaddr_in& addr)
{
repeatConnect:
    if(::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), int(sizeof(sockaddr_in))) == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeatConnect;
        }

        if(connectInProgress())
        {
            return false;
        }

        closeSocketNoThrow(fd);
        if(connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

#if defined(__linux)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    struct sockaddr_in localAddr;
    fdToLocalAddress(fd, localAddr);
    if(compareAddress(addr, localAddr) == 0)
    {
        ConnectionRefusedException ex(__FILE__, __LINE__);
        ex.error = 0; // No appropriate errno
        throw ex;
    }
#endif
    return true;
}

void
IceInternal::doFinishConnect(SOCKET fd)
{
    //
    // Note: we don't close the socket if there's an exception. It's the responsability
    // of the caller to do so.
    //

    //
    // Strange windows bug: The following call to Sleep() is
    // necessary, otherwise no error is reported through
    // getsockopt.
    //
#ifdef _WIN32
    Sleep(0);
#endif

    int val;
    socklen_t len = static_cast<socklen_t>(sizeof(int));
    if(getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    if(val > 0)
    {
#ifdef _WIN32
        WSASetLastError(val);
#else
        errno = val;
#endif
        if(connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else if(connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }

#if defined(__linux)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    struct sockaddr_in localAddr;
    fdToLocalAddress(fd, localAddr);
    struct sockaddr_in remoteAddr;
    if(!fdToRemoteAddress(fd, remoteAddr) && compareAddress(remoteAddr, localAddr) == 0)
    {
        ConnectionRefusedException ex(__FILE__, __LINE__);
        ex.error = 0; // No appropriate errno
        throw ex;
    }
#endif
}

vector<struct sockaddr_in>
IceInternal::getAddresses(const string& host, int port, bool server, bool blocking)
{
    vector<struct sockaddr_in> result;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

#ifdef GUMSTIX
    //
    // Gumstix does not support calling getaddrinfo with empty host.
    //
    if(host.empty())
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if(server)
        {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        }
        result.push_back(addr);
        return result;
    }
#endif

    struct addrinfo* info = 0;
    int retry = 5;

    struct addrinfo hints = { 0 };
    hints.ai_family = PF_INET;

    if(!blocking)
    {
        hints.ai_flags = AI_NUMERICHOST;
    }

    if(server)
    {
        //
        // If host is empty, getaddrinfo will return the wildcard
        // address instead of the loopack address.
        //
        hints.ai_flags |= AI_PASSIVE;
    }

    int rs = 0;
    do
    {
        if(host.empty())
        {
            rs = getaddrinfo(0, "1", &hints, &info); // Get the address of the loopback interface
        }
        else
        {
            rs = getaddrinfo(host.c_str(), 0, &hints, &info);
        }
    }
    while(info == 0 && rs == EAI_AGAIN && --retry >= 0);

    // In theory, getaddrinfo should only return EAI_NONAME if AI_NUMERICHOST is specified and the host name
    // is not a IP address. However on some platforms (e.g. Mac OS X 10.4.x) EAI_NODATA is also returned so
    // we also check for it.
#ifdef EAI_NODATA
    if(!blocking && (rs == EAI_NONAME || rs == EAI_NODATA))
#else
    if(!blocking && rs == EAI_NONAME)
#endif
    {
        return result; // Empty result indicates that a blocking lookup is necessary.
    }
    if(rs != 0)
    {
        DNSException ex(__FILE__, __LINE__);
        ex.error = rs;
        ex.host = host;
        throw ex;
    }

    struct addrinfo* p;
    for(p = info; p != NULL; p = p->ai_next)
    {
        assert(p->ai_family == PF_INET);
        memcpy(&addr, p->ai_addr, p->ai_addrlen);
        struct sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(&addr);
        sin->sin_port = htons(port);

        bool found = false;
        for(unsigned int i = 0; i < result.size(); ++i)
        {
            if(compareAddress(result[i], addr) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            result.push_back(addr);
        }
    }

    freeaddrinfo(info);

    if(result.size() == 0)
    {
        DNSException ex(__FILE__, __LINE__);
        ex.host = host;
        throw ex;
    }

    return result;
}

int
IceInternal::compareAddress(const struct sockaddr_in& addr1, const struct sockaddr_in& addr2)
{
    if(addr1.sin_family < addr2.sin_family)
    {
        return -1;
    }
    else if(addr2.sin_family < addr1.sin_family)
    {
        return 1;
    }

    if(addr1.sin_port < addr2.sin_port)
    {
        return -1;
    }
    else if(addr2.sin_port < addr1.sin_port)
    {
        return 1;
    }

    if(addr1.sin_addr.s_addr < addr2.sin_addr.s_addr)
    {
        return -1;
    }
    else if(addr2.sin_addr.s_addr < addr1.sin_addr.s_addr)
    {
        return 1;
    }

    return 0;
}

void
IceInternal::createPipe(SOCKET fds[2])
{
#ifdef _WIN32

    SOCKET fd = createSocket();
    setBlock(fd, true);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    struct sockaddr_in* addrin = reinterpret_cast<struct sockaddr_in*>(&addr);
    addrin->sin_family = AF_INET;
    addrin->sin_port = htons(0);
    addrin->sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    doBind(fd, addr);
    doListen(fd, 1);

    try
    {
        fds[0] = createSocket();
    }
    catch(...)
    {
        ::closesocket(fd);
        throw;
    }

    try
    {
        setBlock(fds[0], true);
#ifndef NDEBUG
        bool connected = doConnect(fds[0], addr);
        assert(connected);
#else
        doConnect(fds[0], addr);
#endif
    }
    catch(...)
    {
        // fds[0] is closed by doConnect
        ::closesocket(fd);
        throw;
    }

    try
    {
        fds[1] = doAccept(fd);
    }
    catch(...)
    {
        ::closesocket(fds[0]);
        ::closesocket(fd);
        throw;
    }

    ::closesocket(fd);

    try
    {
        setBlock(fds[1], true);
    }
    catch(...)
    {
        ::closesocket(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#else

    if(::pipe(fds) != 0)
    {
        SyscallException ex(__FILE__, __LINE__);
        ex.error = getSystemErrno();
        throw ex;
    }

    try
    {
        setBlock(fds[0], true);
    }
    catch(...)
    {
        // fds[0] is closed by setBlock
        closeSocketNoThrow(fds[1]);
        throw;
    }

    try
    {
        setBlock(fds[1], true);
    }
    catch(...)
    {
        closeSocketNoThrow(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#endif
}

#ifdef _WIN32

string
IceInternal::errorToStringDNS(int error)
{
    return IceUtilInternal::errorToString(error);
}

#else

string
IceInternal::errorToStringDNS(int error)
{
    return gai_strerror(error);
}

#endif

std::string
IceInternal::fdToString(SOCKET fd)
{
    if(fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    struct sockaddr_in localAddr;
    fdToLocalAddress(fd, localAddr);

    struct sockaddr_in remoteAddr;
    bool peerNotConnected = fdToRemoteAddress(fd, remoteAddr);

    string s;
    s += "local address = ";
    s += addrToString(localAddr);
    if(peerNotConnected)
    {
        s += "\nremote address = <not connected>";
    }
    else
    {
        s += "\nremote address = ";
        s += addrToString(remoteAddr);
    }
    return s;
}

void
IceInternal::fdToLocalAddress(SOCKET fd, struct sockaddr_in& addr)
{
    socklen_t len = static_cast<socklen_t>(sizeof(struct sockaddr_in));
    if(getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

bool
IceInternal::fdToRemoteAddress(SOCKET fd, struct sockaddr_in& addr)
{
    bool peerNotConnected = false;
    socklen_t len = static_cast<socklen_t>(sizeof(struct sockaddr_in));
    if(getpeername(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == SOCKET_ERROR)
    {
        if(notConnected())
        {
            peerNotConnected = true;
        }
        else
        {
            closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = getSocketErrno();
            throw ex;
        }
    }
    return peerNotConnected;
}

std::string
IceInternal::addrToString(const struct sockaddr_in& addr)
{
    string s;
    s += inetAddrToString(addr);
    s += ":";
    s += Ice::printfToString("%d", ntohs(addr.sin_port));
    return s;
}

vector<string>
IceInternal::getHostsForEndpointExpand(const string& host)
{
    vector<string> hosts;
    if(host.empty() || host == "0.0.0.0")
    {
        vector<struct sockaddr_in> addrs = getLocalAddresses();
        for(vector<struct sockaddr_in>::const_iterator p = addrs.begin(); p != addrs.end(); ++p)
        {
            hosts.push_back(inetAddrToString((*p)));
        }

        if(hosts.empty())
        {
            hosts.push_back("127.0.0.1");
        }
    }
    return hosts; // An empty host list indicates to just use the given host.
}

bool
IceInternal::acceptInterrupted()
{
    if(interrupted())
    {
        return true;
    }

#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNABORTED ||
           error == WSAECONNRESET ||
           error == WSAETIMEDOUT;
#else
    return errno == ECONNABORTED ||
           errno == ECONNRESET ||
           errno == ETIMEDOUT;
#endif
}

SOCKET
IceInternal::doAccept(SOCKET fd)
{
#ifdef _WIN32                   
    SOCKET ret;
#else       
        int ret;    
#endif 

repeatAccept:
    if((ret = ::accept(fd, 0, 0)) == INVALID_SOCKET)
    {
        if(acceptInterrupted())
        {
            goto repeatAccept;
        }

        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }

    setTcpNoDelay(ret);
    setKeepAlive(ret);
    return ret;
}

void
IceInternal::doListen(SOCKET fd, int backlog)
{
repeatListen:
    if(::listen(fd, backlog) == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeatListen;
        }
        
        closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = getSocketErrno();
        throw ex;
    }
}

void
IceInternal::setTcpBufSize(SOCKET fd, const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& logger)
{
    assert(fd != INVALID_SOCKET);

    //
    // By default, on Windows we use a 64KB buffer size. On Unix
    // platforms, we use the system defaults.
    //
#ifdef _WIN32
    const int dfltBufSize = 64 * 1024;
#else
    const int dfltBufSize = 0;
#endif
    Int sizeRequested;

#ifndef _WIN32_WCE
    //
    // Sockect option is not available on CE to set receive buffer size.
    //
    sizeRequested = properties->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
    if(sizeRequested > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setRecvBufferSize(fd, sizeRequested);
        int size = getRecvBufferSize(fd);
        if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
        {
            Warning out(logger);
            out << printfToString("TCP receive buffer size: requested size of %d adjusted to %d", sizeRequested, size);
        }
    }
#endif //_WIN32_WCE


    sizeRequested = properties->getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);
    if(sizeRequested > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setSendBufferSize(fd, sizeRequested);
        int size = getSendBufferSize(fd);
        if(size < sizeRequested) // Warn if the size that was set is less than the requested size.
        {
            Warning out(logger);
            out << printfToString("TCP send buffer size: requested size of %d adjusted to %d", sizeRequested, size);
        }
    }
}
