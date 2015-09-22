// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/Instance.h>
#include <IceE/MutexPtrLock.h>
#include <IceE/TraceLevels.h>
#include <IceE/DefaultsAndOverrides.h>
#ifdef ICEE_HAS_ROUTER
#    include <IceE/RouterInfo.h>
#    include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#    include <IceE/LocatorInfo.h>
#    include <IceE/Locator.h>
#endif
#include <IceE/ReferenceFactory.h>
#include <IceE/ProxyFactory.h>
#include <IceE/ThreadPool.h>
#include <IceE/SelectorThread.h>
#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Properties.h>
#include <IceE/LoggerI.h>
#include <IceE/EndpointFactory.h>
#include <IceE/Endpoint.h>
#include <IceE/ObjectFactoryManagerI.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/ObjectAdapterFactory.h>
#endif
#ifdef ICEE_HAS_AMI
#    include <IceE/RetryQueue.h>
#endif
#include <IceE/Mutex.h>
#include <IceE/StringUtil.h>

#ifdef _WIN32
#   include <winsock2.h>
#   ifndef _WIN32_WCE
#      include <process.h>
#   endif
#else
#   include <signal.h>
#   include <pwd.h>
#   include <sys/types.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

static IceUtil::Mutex* staticMutex = 0;
static bool oneOffDone = false;
static int instanceCount = 0;
static bool printProcessIdDone = false;

namespace IceUtil
{

extern bool nullHandleAbort;

}

namespace
{

class Init
{
public:

    Init()
    {
        staticMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete staticMutex;
        staticMutex = 0;
    }
};
Init init;

}

IceUtil::Shared* IceInternal::upCast(Instance* p) { return p; }

bool
IceInternal::Instance::destroyed() const
{
    IceUtil::RecMutex::Lock sync(*this);
    return _state == StateDestroyed;
}

TraceLevelsPtr
IceInternal::Instance::traceLevels() const
{
    // No mutex lock, immutable.
    assert(_traceLevels);
    return _traceLevels;
}

DefaultsAndOverridesPtr
IceInternal::Instance::defaultsAndOverrides() const
{
    // No mutex lock, immutable.
    assert(_defaultsAndOverrides);
    return _defaultsAndOverrides;
}

#ifdef ICEE_HAS_ROUTER

RouterManagerPtr
IceInternal::Instance::routerManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_routerManager);
    return _routerManager;
}

#endif

#ifdef ICEE_HAS_LOCATOR

LocatorManagerPtr
IceInternal::Instance::locatorManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_locatorManager);
    return _locatorManager;
}

#endif

ReferenceFactoryPtr
IceInternal::Instance::referenceFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_referenceFactory);
    return _referenceFactory;
}

ProxyFactoryPtr
IceInternal::Instance::proxyFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_proxyFactory);
    return _proxyFactory;
}

OutgoingConnectionFactoryPtr
IceInternal::Instance::outgoingConnectionFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_outgoingConnectionFactory);
    return _outgoingConnectionFactory;
}

ObjectFactoryManagerPtr
IceInternal::Instance::servantFactoryManager() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_servantFactoryManager)
    {
        const_cast<Instance&>(*this)._servantFactoryManager = new ObjectFactoryManagerI();
    }

    assert(_servantFactoryManager);
    return _servantFactoryManager;
}

#ifndef ICEE_PURE_CLIENT
ObjectAdapterFactoryPtr
IceInternal::Instance::objectAdapterFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_objectAdapterFactory);
    return _objectAdapterFactory;
}
#endif

#ifdef ICEE_HAS_AMI
RetryQueuePtr
IceInternal::Instance::retryQueue() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    assert(_retryQueue);
    return _retryQueue;
}

EndpointHostResolverPtr
IceInternal::Instance::endpointHostResolver()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_endpointHostResolver);
    return _endpointHostResolver;
}
#endif

EndpointFactoryPtr
IceInternal::Instance::endpointFactory() const
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    assert(_endpointFactory);
    return _endpointFactory;
}

#ifdef ICEE_HAS_BATCH
void
IceInternal::Instance::flushBatchRequests()
{

    OutgoingConnectionFactoryPtr connectionFactory;
#ifndef ICEE_PURE_CLIENT
    ObjectAdapterFactoryPtr adapterFactory;
#endif

    {
        IceUtil::RecMutex::Lock sync(*this);

        if(_state == StateDestroyed)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        connectionFactory = _outgoingConnectionFactory;
#ifndef ICEE_PURE_CLIENT
        adapterFactory = _objectAdapterFactory;
#endif
    }

    connectionFactory->flushBatchRequests();
#ifndef ICEE_PURE_CLIENT
    adapterFactory->flushBatchRequests();
#endif

}
#endif

ThreadPoolPtr
IceInternal::Instance::clientThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_clientThreadPool);
    return _clientThreadPool;
}

#ifndef ICEE_PURE_CLIENT
ThreadPoolPtr
IceInternal::Instance::serverThreadPool()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    if(!_serverThreadPool) // Lazy initialization.
    {
        int timeout = _initData.properties->getPropertyAsInt("Ice.ServerIdleTime");
        _serverThreadPool = new ThreadPool(this, "Ice.ThreadPool.Server", timeout);
    }

    return _serverThreadPool;
}
#endif

SelectorThreadPtr
IceInternal::Instance::selectorThread()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_selectorThread);
    return _selectorThread;
}

IceUtil::TimerPtr
IceInternal::Instance::timer()
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(_timer);
    return _timer;
}

Identity
IceInternal::Instance::stringToIdentity(const string& s) const
{
    Identity ident;

    //
    // Find unescaped separator.
    //
    string::size_type slash = string::npos, pos = 0;
    while((pos = s.find('/', pos)) != string::npos)
    {
        if(pos == 0 || s[pos - 1] != '\\')
        {
            if(slash == string::npos)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
            }
        }
        pos++;
    }

    if(slash == string::npos)
    {
        if(!IceUtilInternal::unescapeString(s, 0, s.size(), ident.name))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
    }
    else
    {
        if(!IceUtilInternal::unescapeString(s, 0, slash, ident.category))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            if(!IceUtilInternal::unescapeString(s, slash + 1, s.size(), ident.name))
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
            }
        }
    }

#ifdef ICEE_HAS_WSTRING
    if(_initData.stringConverter)
    {
        string tmpString;
        _initData.stringConverter->fromUTF8(reinterpret_cast<const Byte*>(ident.name.data()),
                                            reinterpret_cast<const Byte*>(ident.name.data() + ident.name.size()),
                                            tmpString);
        ident.name = tmpString;

        _initData.stringConverter->fromUTF8(reinterpret_cast<const Byte*>(ident.category.data()),
                                           reinterpret_cast<const Byte*>(ident.category.data() + ident.category.size()),
                                           tmpString);
        ident.category = tmpString;
    }
#endif

    return ident;
}

string
IceInternal::Instance::identityToString(const Identity& ident) const
{
    string name = ident.name;
    string category = ident.category;
#ifdef ICEE_HAS_WSTRING
    if(_initData.stringConverter)
    {
        UTF8BufferI buffer;
        Byte* last = _initData.stringConverter->toUTF8(ident.name.data(), ident.name.data() + ident.name.size(),
                                                       buffer);
        name = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());

        buffer.reset();
        last = _initData.stringConverter->toUTF8(ident.category.data(), ident.category.data() + ident.category.size(),
                                                 buffer);
        category = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
    }
#endif
    if(category.empty())
    {
        return IceUtilInternal::escapeString(name, "/");
    }
    else
    {
        return IceUtilInternal::escapeString(category, "/") + '/' + IceUtilInternal::escapeString(name, "/");
    }
}

#ifdef ICEE_HAS_LOCATOR
void
IceInternal::Instance::setDefaultLocator(const Ice::LocatorPrx& defaultLocator)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultLocator(defaultLocator);
}
#endif

#ifdef ICEE_HAS_ROUTER
void
IceInternal::Instance::setDefaultRouter(const Ice::RouterPrx& defaultRouter)
{
    IceUtil::RecMutex::Lock sync(*this);

    if(_state == StateDestroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _referenceFactory = _referenceFactory->setDefaultRouter(defaultRouter);
}
#endif

IceInternal::Instance::Instance(const CommunicatorPtr& communicator, const InitializationData& initData) :
    _state(StateActive),
    _initData(initData),
    _messageSizeMax(0)
{
    try
    {
        __setNoDelete(true);

        {
            IceUtilInternal::MutexPtrLock lock(staticMutex);
            instanceCount++;

            if(!oneOffDone)
            {
                //
                // StdOut and StdErr redirection
                //

                string stdOutFilename = _initData.properties->getProperty("Ice.StdOut");
                string stdErrFilename = _initData.properties->getProperty("Ice.StdErr");

                if(stdOutFilename != "")
                {
                    FILE * file;
#ifdef _WIN32_WCE
                    wchar_t* wtext = new wchar_t[sizeof(wchar_t) * stdOutFilename.length()];
                    mbstowcs(wtext, stdOutFilename.c_str(), stdOutFilename.length());
                    file = _wfreopen(wtext, L"a", stdout);
                    delete wtext;
#else
                    file = freopen(stdOutFilename.c_str(), "a", stdout);
#endif
                    if(file == 0)
                    {
                        SyscallException ex(__FILE__, __LINE__);
                        ex.error = getSystemErrno();
                        throw ex;
                    }
                }

                if(stdErrFilename != "")
                {
                    FILE* file;
#ifdef _WIN32_WCE
                    wchar_t* wtext = new wchar_t[sizeof(wchar_t) * stdErrFilename.length()];
                    mbstowcs(wtext, stdErrFilename.c_str(), stdErrFilename.length());
                    file = _wfreopen(wtext, L"a", stderr);
                    delete wtext;
#else
                    file = freopen(stdErrFilename.c_str(), "a", stderr);
#endif
                    if(file == 0)
                    {
                        SyscallException ex(__FILE__, __LINE__);
                        ex.error = getSystemErrno();
                        throw ex;
                    }
                }

                if(_initData.properties->getPropertyAsInt("Ice.NullHandleAbort") > 0)
                {
                    IceUtil::nullHandleAbort = true;
                }

#ifndef _WIN32
                string newUser = _initData.properties->getProperty("Ice.ChangeUser");
                if(!newUser.empty())
                {
                    struct passwd* pw = getpwnam(newUser.c_str());
                    if(!pw)
                    {
                        SyscallException ex(__FILE__, __LINE__);
                        ex.error = getSystemErrno();
                        throw ex;
                    }

                    if(setgid(pw->pw_gid) == -1)
                    {
                        SyscallException ex(__FILE__, __LINE__);
                        ex.error = getSystemErrno();
                        throw ex;
                    }

                    if(setuid(pw->pw_uid) == -1)
                    {
                        SyscallException ex(__FILE__, __LINE__);
                        ex.error = getSystemErrno();
                        throw ex;
                    }
                }
#endif
                oneOffDone = true;
            }

            if(instanceCount == 1)
            {

#ifdef _WIN32
                WORD version = MAKEWORD(1, 1);
                WSADATA data;
                if(WSAStartup(version, &data) != 0)
                {
                    SocketException ex(__FILE__, __LINE__);
                    ex.error = WSAGetLastError();
                    throw ex;
                }
#endif

#ifndef _WIN32
                struct sigaction action;
                action.sa_handler = SIG_IGN;
                sigemptyset(&action.sa_mask);
                action.sa_flags = 0;
                sigaction(SIGPIPE, &action, 0);
#endif
            }
        }

        if(!_initData.logger)
        {
            _initData.logger = new LoggerI(_initData.properties->getProperty("Ice.ProgramName"));
        }

        const_cast<TraceLevelsPtr&>(_traceLevels) = new TraceLevels(_initData.properties);

        const_cast<DefaultsAndOverridesPtr&>(_defaultsAndOverrides) = new DefaultsAndOverrides(_initData.properties);

        {
            static const int defaultMessageSizeMax = 1024;
            Int num = _initData.properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
            if(num < 1)
            {
                const_cast<size_t&>(_messageSizeMax) = defaultMessageSizeMax * 1024; // Ignore non-sensical values.
            }
            else if(static_cast<size_t>(num) > (size_t)(0x7fffffff / 1024))
            {
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(0x7fffffff);
            }
            else
            {
                // Property is in kilobytes, _messageSizeMax in bytes.
                const_cast<size_t&>(_messageSizeMax) = static_cast<size_t>(num) * 1024;
            }
        }

#ifdef ICEE_HAS_ROUTER
        _routerManager = new RouterManager;
#endif

#ifdef ICEE_HAS_LOCATOR
        _locatorManager = new LocatorManager;
#endif

        _referenceFactory = new ReferenceFactory(this, communicator);

        _proxyFactory = new ProxyFactory(this);

        _endpointFactory = new EndpointFactory(this);

        _outgoingConnectionFactory = new OutgoingConnectionFactory(this);

#ifndef ICEE_PURE_CLIENT
        _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);
#endif

#ifdef ICEE_HAS_AMI
        _retryQueue = new RetryQueue(this);
        try
        {
            _endpointHostResolver = new EndpointHostResolver(this);
        }
        catch(const IceUtil::Exception& ex)
        {
            Error out(_initData.logger);
            out << "cannot create thread for endpoint host resolver:\n" << ex.toString();
            throw;
        }
#endif

        try
        {
            if(_initData.properties->getProperty("Ice.ThreadPriority") != "")
            {
                _timer = new IceUtil::Timer(_initData.properties->getPropertyAsInt("Ice.ThreadPriority"));
            }
            else
            {
                _timer = new IceUtil::Timer;
            }
        }
        catch(const IceUtil::Exception& ex)
        {
            Error out(_initData.logger);
            out << "cannot create thread for timer:\n" << ex.toString();
            throw;
        }

        _clientThreadPool = new ThreadPool(this, "Ice.ThreadPool.Client", 0);

        _selectorThread = new SelectorThread(this);

#ifdef ICEE_HAS_WSTRING
        if(!_initData.wstringConverter)
        {
            _initData.wstringConverter = new UnicodeWstringConverter();
        }
#endif

        __setNoDelete(false);
    }
    catch(...)
    {
        {
            IceUtilInternal::MutexPtrLock lock(staticMutex);
            --instanceCount;
        }
        destroy();
        __setNoDelete(false);
        throw;
    }
}

IceInternal::Instance::~Instance()
{
    assert(_state == StateDestroyed);
    assert(!_referenceFactory);
    assert(!_proxyFactory);
    assert(!_outgoingConnectionFactory);
    assert(!_servantFactoryManager);
#ifndef ICEE_PURE_CLIENT
    assert(!_objectAdapterFactory);
#endif
#ifdef ICEE_HAS_AMI
    assert(!_retryQueue);
    assert(!_endpointHostResolver);
#endif
#ifdef ICEE_HAS_ROUTER
    assert(!_routerManager);
#endif
#ifdef ICEE_HAS_LOCATOR
    assert(!_locatorManager);
#endif
    assert(!_endpointFactory);

    IceUtilInternal::MutexPtrLock lock(staticMutex);
    if(--instanceCount == 0)
    {
#ifdef _WIN32
        WSACleanup();
#endif

#ifndef _WIN32
        struct sigaction action;
        action.sa_handler = SIG_DFL;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGPIPE, &action, 0);
#endif
    }
}

void
IceInternal::Instance::finishSetup(int& argc, char* argv[])
{
    //
    // Get default router and locator proxies. Don't move this
    // initialization before the plug-in initialization!!! The proxies
    // might depend on endpoint factories to be installed by plug-ins.
    //
#ifdef ICEE_HAS_ROUTER
    RouterPrx router = RouterPrx::uncheckedCast(_proxyFactory->propertyToProxy("Ice.Default.Router"));
    if(router)
    {
        _referenceFactory = _referenceFactory->setDefaultRouter(router);
    }
#endif

#ifdef ICEE_HAS_LOCATOR
    LocatorPrx locator = LocatorPrx::uncheckedCast(_proxyFactory->propertyToProxy("Ice.Default.Locator"));
    if(locator)
    {
        _referenceFactory = _referenceFactory->setDefaultLocator(locator);
    }
#endif

#ifndef _WIN32_WCE
    //
    // Show process id if requested (but only once).
    //
    bool printProcessId = false;
    if(!printProcessIdDone && _initData.properties->getPropertyAsInt("Ice.PrintProcessId") > 0)
    {
        //
        // Safe double-check locking (no dependent variable!)
        //
        IceUtilInternal::MutexPtrLock lock(staticMutex);
        printProcessId = !printProcessIdDone;

        //
        // We anticipate: we want to print it once, and we don't care when.
        //
        printProcessIdDone = true;
    }

    if(printProcessId)
    {
#ifdef _WIN32
        printf("%d\n", _getpid());
#else
        printf("%d\n", getpid());
#endif
        fflush(stdout);
    }
#endif
}

void
IceInternal::Instance::destroy()
{
    {
        IceUtil::RecMutex::Lock sync(*this);

        //
        // If the _state is not StateActive then the instance is
        // either being destroyed, or has already been destroyed.
        //
        if(_state != StateActive)
        {
            return;
        }

        //
        // We cannot set state to StateDestroyed otherwise instance
        // methods called during the destroy process (such as
        // outgoingConnectionFactory() from
        // ObjectAdapterI::deactivate() will cause an exception.
        //
        _state = StateDestroyInProgress;
    }

#ifndef ICEE_PURE_CLIENT
    if(_objectAdapterFactory)
    {
        _objectAdapterFactory->shutdown();
    }

    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->destroy();
    }

    if(_objectAdapterFactory)
    {
        _objectAdapterFactory->destroy();
    }

    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->waitUntilFinished();
    }
#else
    if(_outgoingConnectionFactory)
    {
        _outgoingConnectionFactory->destroy();
        _outgoingConnectionFactory->waitUntilFinished();
    }
#endif

#ifdef ICEE_HAS_AMI
    if(_retryQueue)
    {
        _retryQueue->destroy();
    }
#endif

#ifndef ICEE_PURE_CLIENT
    ThreadPoolPtr serverThreadPool;
#endif
    ThreadPoolPtr clientThreadPool;
    SelectorThreadPtr selectorThread;
#ifdef ICEE_HAS_AMI
    EndpointHostResolverPtr endpointHostResolver;
#endif

    {
        IceUtil::RecMutex::Lock sync(*this);

#ifndef ICEE_PURE_CLIENT
        _objectAdapterFactory = 0;
#endif
        _outgoingConnectionFactory = 0;
#ifdef ICEE_HAS_AMI
        _retryQueue = 0;
#endif

#ifndef ICEE_PURE_CLIENT
        if(_serverThreadPool)
        {
            _serverThreadPool->destroy();
            std::swap(_serverThreadPool, serverThreadPool);
        }
#endif
        
        if(_clientThreadPool)
        {
            _clientThreadPool->destroy();
            std::swap(_clientThreadPool, clientThreadPool);
        }

        if(_selectorThread)
        {
            _selectorThread->destroy();
            std::swap(selectorThread, _selectorThread);
        }

#ifdef ICEE_HAS_AMI
        if(_endpointHostResolver)
        {
            _endpointHostResolver->destroy();
            std::swap(endpointHostResolver, _endpointHostResolver);
        }
#endif

        if(_timer)
        {
            _timer->destroy();
            _timer = 0;
        }

        if(_servantFactoryManager)
        {
            _servantFactoryManager->destroy();
            _servantFactoryManager = 0;
        }

        //_referenceFactory->destroy(); // No destroy function defined.
        _referenceFactory = 0;

        //_proxyFactory->destroy(); // No destroy function defined.
        _proxyFactory = 0;

#ifdef ICEE_HAS_ROUTER
        if(_routerManager)
        {
            _routerManager->destroy();
            _routerManager = 0;
        }
#endif

#ifdef ICEE_HAS_LOCATOR
        if(_locatorManager)
        {
            _locatorManager->destroy();
            _locatorManager = 0;
        }
#endif

        if(_endpointFactory)
        {
            _endpointFactory->destroy();
            _endpointFactory = 0;
        }

        _state = StateDestroyed;
    }

    //
    // Join with threads outside the synchronization.
    //
    if(clientThreadPool)
    {
        clientThreadPool->joinWithAllThreads();
    }
#ifndef ICEE_PURE_CLIENT
    if(serverThreadPool)
    {
        serverThreadPool->joinWithAllThreads();
    }
#endif
    if(selectorThread)
    {
        selectorThread->joinWithThread();
    }
#ifdef ICEE_HAS_AMI
    if(endpointHostResolver)
    {
        endpointHostResolver->getThreadControl().join();
    }
#endif
}

#ifdef ICEE_HAS_WSTRING
IceInternal::UTF8BufferI::UTF8BufferI() :
    _buffer(0),
    _offset(0)
{
}

IceInternal::UTF8BufferI::~UTF8BufferI()
{
    free(_buffer);
}

Byte*
IceInternal::UTF8BufferI::getMoreBytes(size_t howMany, Byte* firstUnused)
{
    if(_buffer == 0)
    {
        _buffer = (Byte*)malloc(howMany);
    }
    else
    {
        assert(firstUnused != 0);
        _offset = firstUnused - _buffer;
        _buffer = (Byte*)realloc(_buffer, _offset + howMany);
    }

    return _buffer + _offset;
}

Byte*
IceInternal::UTF8BufferI::getBuffer()
{
    return _buffer;
}

void
IceInternal::UTF8BufferI::reset()
{
    free(_buffer);
    _buffer = 0;
    _offset = 0;
}
#endif
