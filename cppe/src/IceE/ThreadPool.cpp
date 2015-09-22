// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/ThreadPool.h>
#include <IceE/EventHandler.h>
#include <IceE/Network.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Functional.h>
#include <IceE/Protocol.h>
#ifndef ICEE_PURE_CLIENT
#   include <IceE/ObjectAdapterFactory.h>
#endif
#include <IceE/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

ICE_DECLSPEC_EXPORT IceUtil::Shared* IceInternal::upCast(ThreadPool* p) { return p; }

IceInternal::ThreadPool::ThreadPool(const InstancePtr& instance, const string& prefix, int timeout) :
    _instance(instance),
    _destroyed(false),
    _prefix(prefix),
    _selector(instance, timeout),
    _size(0),
    _sizeMax(0),
    _sizeWarn(0),
    _serialize(_instance->initializationData().properties->getPropertyAsInt(_prefix + ".Serialize") > 0),
    _stackSize(0),
    _running(0),
    _inUse(0),
    _load(1.0),
    _promote(true),
    _hasPriority(false),
    _priority(0)
{
    //
    // We use just one thread as the default. This is the fastest
    // possible setting, still allows one level of nesting, and
    // doesn't require to make the servants thread safe.
    //
    Ice::PropertiesPtr properties = _instance->initializationData().properties;

    int size = properties->getPropertyAsIntWithDefault(_prefix + ".Size", 1);
    if(size < 1)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".Size < 1; Size adjusted to 1";
        size = 1;
    }

    int sizeMax = properties->getPropertyAsIntWithDefault(_prefix + ".SizeMax", size);
    if(sizeMax < size)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeMax < " << _prefix << ".Size; SizeMax adjusted to Size (" << size << ")";
        sizeMax = size;
    }

    int sizeWarn = properties->getPropertyAsIntWithDefault(_prefix + ".SizeWarn", sizeMax * 80 / 100);
    if(sizeWarn > sizeMax)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".SizeWarn > " << _prefix << ".SizeMax; adjusted SizeWarn to SizeMax (" << sizeMax << ")";
        sizeWarn = sizeMax;
    }

    const_cast<int&>(_size) = size;
    const_cast<int&>(_sizeMax) = sizeMax;
    const_cast<int&>(_sizeWarn) = sizeWarn;

    int stackSize = properties->getPropertyAsInt(_prefix + ".StackSize");
    if(stackSize < 0)
    {
        Warning out(_instance->initializationData().logger);
        out << _prefix << ".StackSize < 0; Size adjusted to OS default";
        stackSize = 0;
    }
    const_cast<size_t&>(_stackSize) = static_cast<size_t>(stackSize);

    const_cast<bool&>(_hasPriority) = properties->getProperty(_prefix + ".ThreadPriority") != "";
    const_cast<int&>(_priority) = properties->getPropertyAsInt(_prefix + ".ThreadPriority");
    if(!_hasPriority)
    {
        const_cast<bool&>(_hasPriority) = properties->getProperty("Ice.ThreadPriority") != "";
        const_cast<int&>(_priority) = properties->getPropertyAsInt("Ice.ThreadPriority");
    }
    __setNoDelete(true);
    try
    {
        for(int i = 0 ; i < _size ; ++i)
        {
            IceUtil::ThreadPtr thread = new EventHandlerThread(this);
            if(_hasPriority)
            {
                thread->start(_stackSize, _priority);
            }
            else
            {
                thread->start(_stackSize);
            }
            _threads.push_back(thread);
            ++_running;
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        {
            Error out(_instance->initializationData().logger);
            out << "cannot create thread for `" << _prefix << "':\n" << ex.toString();
        }

        destroy();
        joinWithAllThreads();
        __setNoDelete(false);
        throw;
    }
    catch(...)
    {
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

IceInternal::ThreadPool::~ThreadPool()
{
    assert(_destroyed);
}

void
IceInternal::ThreadPool::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    _selector.setInterrupt();
}

void
IceInternal::ThreadPool::incFdsInUse()
{
#ifdef ICE_USE_SELECT
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _selector.incFdsInUse();
#endif
}

void
IceInternal::ThreadPool::decFdsInUse()
{
#ifdef ICE_USE_SELECT
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _selector.decFdsInUse();
#endif
}

void
IceInternal::ThreadPool::_register(const EventHandlerPtr& handler)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed && handler->_fd != INVALID_SOCKET);
    if(!handler->_registered)
    {
        if(!handler->_serializing)
        {
            _selector.add(handler.get(), NeedRead);
        }
        handler->_registered = true;
    }
}

void
IceInternal::ThreadPool::unregister(const EventHandlerPtr& handler)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed && handler->_fd != INVALID_SOCKET);
    if(handler->_registered)
    {
        if(!handler->_serializing)
        {
            _selector.remove(handler.get(), NeedRead);
        }
        handler->_registered = false;
    }
}

void
IceInternal::ThreadPool::finish(const EventHandlerPtr& handler)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(!_destroyed && handler->_fd != INVALID_SOCKET);
    if(handler->_registered)
    {
        if(!handler->_serializing)
        {
            _selector.remove(handler.get(), NeedRead);
        }
        handler->_registered = false;
    }
    _finished.push_back(handler);
    _selector.setInterrupt();
}

void
IceInternal::ThreadPool::execute(const ThreadPoolWorkItemPtr& workItem)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_destroyed)
    {
        throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _workItems.push_back(workItem);
    _selector.setInterrupt();
}

void
IceInternal::ThreadPool::promoteFollower(EventHandler* handler)
{
    if(_sizeMax > 1)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        if(_serialize && handler)
        {
            handler->_serializing = true;
            if(handler->_registered)
            {
                _selector.remove(handler, NeedRead, true); // No interrupt, no thread is blocked on select().
            }
        }

        assert(!_promote);
        _promote = true;
        notify();

        if(!_destroyed)
        {
            assert(_inUse >= 0);
            ++_inUse;

            if(_inUse == _sizeWarn)
            {
                Warning out(_instance->initializationData().logger);
                out << "thread pool `" << _prefix << "' is running low on threads\n"
                    << "Size=" << _size << ", " << "SizeMax=" << _sizeMax << ", " << "SizeWarn=" << _sizeWarn;
            }

            assert(_inUse <= _running);
            if(_inUse < _sizeMax && _inUse == _running)
            {
                try
                {
                    IceUtil::ThreadPtr thread = new EventHandlerThread(this);
                    if(_hasPriority)
                    {
                        thread->start(_stackSize, _priority);
                    }
                    else
                    {
                        thread->start(_stackSize);
                    }
                    _threads.push_back(thread);
                    ++_running;
                }
                catch(const IceUtil::Exception& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "cannot create thread for `" << _prefix << "':\n" << ex.toString();
                }
            }
        }
    }
}

void
IceInternal::ThreadPool::joinWithAllThreads()
{
    //
    // _threads is immutable after destroy() has been called,
    // therefore no synchronization is needed. (Synchronization
    // wouldn't be possible here anyway, because otherwise the other
    // threads would never terminate.)
    //
    assert(_destroyed);
    for(vector<IceUtil::ThreadPtr>::iterator p = _threads.begin(); p != _threads.end(); ++p)
    {
        (*p)->getThreadControl().join();
    }
}

string
IceInternal::ThreadPool::prefix() const
{
    return _prefix;
}

bool
IceInternal::ThreadPool::run()
{
    ThreadPoolPtr self = this;

    if(_sizeMax > 1)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        while(!_promote)
        {
            wait();
        }

        _promote = false;
    }

    while(true)
    {
        int ret;
        try
        {
            ret = _selector.select();
        }
        catch(const Ice::LocalException& ex)
        {
            Error out(_instance->initializationData().logger);
            out << "exception in `" << _prefix << "':\n" << ex.toString();
            continue;
        }

        EventHandlerPtr handler;
        ThreadPoolWorkItemPtr workItem;
        bool finished = false;
        bool shutdown = false;

        if(ret == 0) // We initiate a shutdown if there is a thread pool timeout.
        {
            shutdown = true;
        }
        else
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            if(_selector.isInterrupted())
            {
                if(_selector.processInterrupt())
                {
                    continue;
                }

                //
                // There are three possiblities for an interrupt:
                //
                // 1. An event handler is being finished (closed).
                //
                // 2. The thread pool has been destroyed.
                //
                // 3. A work item has been schedulded.
                //
                if(!_finished.empty())
                {
                    _selector.clearInterrupt();
                    handler = _finished.front();
                    _finished.pop_front();
                    finished = true;
                }
                else if(!_workItems.empty())
                {
                    //
                    // Work items must be executed first even if the thread pool is destroyed.
                    //
                    _selector.clearInterrupt();
                    workItem = _workItems.front();
                    _workItems.pop_front();
                }
                else if(_destroyed)
                {
                    //
                    // Don't clear the interrupt if destroyed, so that the other threads exit as well.
                    //
                    return true;
                }
                else
                {
                    assert(false);
                }
            }
            else
            {
                handler = _selector.getNextSelected();
                if(!handler)
                {
                    continue;
                }
            }
        }

        //
        // Now we are outside the thread synchronization.
        //
        if(shutdown)
        {
            //
            // Initiate server shutdown.
            //
#ifndef ICEE_PURE_CLIENT
            ObjectAdapterFactoryPtr factory;
            try
            {
                factory = _instance->objectAdapterFactory();
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
                continue;
            }

            promoteFollower();
            factory->shutdown();

            //
            // No "continue", because we want shutdown to be done in
            // its own thread from this pool. Therefore we called
            // promoteFollower().
            //
#else
            continue;
#endif

        }
        else if(workItem)
        {
            try
            {
                //
                // "self" is faster than "this", as the reference
                // count is not modified.
                //
                workItem->execute(self);
            }
            catch(const LocalException& ex)
            {
                Error out(_instance->initializationData().logger);
                out << "exception in `" << _prefix << "' while calling execute():\n" << ex.toString();
            }

            //
            // No "continue", because we want execute() to be
            // called in its own thread from this pool. Note that
            // this means that execute() must call
            // promoteFollower().
            //
        }
        else
        {
            assert(handler);

            if(finished)
            {
                //
                // Notify a handler about it's removal from the thread pool.
                //
                try
                {
                    //
                    // "self" is faster than "this", as the reference count is not modified.
                    //
                    handler->finished(self);
                }
                catch(const LocalException& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "exception in `" << _prefix << "' while calling finished():\n"
                        << ex.toString() << '\n' << handler->toString();
                }

                //
                // No "continue", because we want finished() to be
                // called in its own thread from this pool. Note that
                // this means that finished() must call
                // promoteFollower().
                //
            }
            else
            {
                //
                // If the handler is "readable", try to read a message.
                //
                BasicStream stream(_instance.get(), _instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
                                   , _instance->initializationData().stringConverter,
                                   _instance->initializationData().wstringConverter
#endif
                                  );
                if(handler->readable())
                {
                    try
                    {
                        if(!read(handler))
                        {
                            continue; // Can't read without blocking.
                        }
                    }
                    catch(const TimeoutException&)
                    {
                        assert(false); // This shouldn't occur as we only perform non-blocking reads.
                        continue;
                    }
                    catch(const SocketException& ex)
                    {
                        handler->exception(ex);
                        continue;
                    }
                    catch(const LocalException& ex)
                    {
                        handler->exception(ex);
                        continue;
                    }

                    stream.swap(handler->_stream);
                    assert(stream.i == stream.b.end());
                }

                //
                // Provide a new mesage to the handler.
                //
                try
                {
                    //
                    // "self" is faster than "this", as the reference count is not modified.
                    //
                    handler->message(stream, self);
                }
                catch(const LocalException& ex)
                {
                    Error out(_instance->initializationData().logger);
                    out << "exception in `" << _prefix << "' while calling message():\n"
                        << ex.toString() << '\n' << handler->toString();
                }

                //
                // No "continue", because we want message() to be
                // called in its own thread from this pool. Note that
                // this means that message() must call
                // promoteFollower().
                //
            }
        }

        if(_sizeMax > 1)
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

            if(!_destroyed)
            {
                if(_serialize && handler && handler->_serializing)
                {
                    if(handler->_registered)
                    {
                        // No interrupt if no thread is blocked on select (_promote == true)
                        _selector.add(handler.get(), NeedRead, _promote);
                    }
                    handler->_serializing = false;
                }

                //
                // First we reap threads that have been destroyed before.
                //
                int sz = static_cast<int>(_threads.size());
                assert(_running <= sz);
                if(_running < sz)
                {
                    vector<IceUtil::ThreadPtr>::iterator start =
                        partition(_threads.begin(), _threads.end(), IceUtil::constMemFun(&IceUtil::Thread::isAlive));

                    for(vector<IceUtil::ThreadPtr>::iterator p = start; p != _threads.end(); ++p)
                    {
                        (*p)->getThreadControl().join();
                    }

                    _threads.erase(start, _threads.end());
                }

                //
                // Now we check if this thread can be destroyed, based
                // on a load factor.
                //

                //
                // The load factor jumps immediately to the number of
                // threads that are currently in use, but decays
                // exponentially if the number of threads in use is
                // smaller than the load factor. This reflects that we
                // create threads immediately when they are needed,
                // but want the number of threads to slowly decline to
                // the configured minimum.
                //
                double inUse = static_cast<double>(_inUse);
                if(_load < inUse)
                {
                    _load = inUse;
                }
                else
                {
                    const double loadFactor = 0.05; // TODO: Configurable?
                    const double oneMinusLoadFactor = 1 - loadFactor;
                    _load = _load * oneMinusLoadFactor + inUse * loadFactor;
                }

                if(_running > _size)
                {
                    int load = static_cast<int>(_load + 0.5);

                    //
                    // We add one to the load factor because on
                    // additional thread is needed for select().
                    //
                    if(load + 1 < _running)
                    {
                        assert(_inUse > 0);
                        --_inUse;

                        assert(_running > 0);
                        --_running;

                        return false;
                    }
                }

                assert(_inUse > 0);
                --_inUse;
            }

            while(!_promote)
            {
                wait();
            }

            _promote = false;
        }
    }
}

bool
IceInternal::ThreadPool::read(const EventHandlerPtr& handler)
{
    BasicStream& stream = handler->_stream;

    if(stream.b.size() == 0)
    {
        stream.b.resize(headerSize);
        stream.i = stream.b.begin();
    }

    if(stream.i != stream.b.end())
    {
        if(!handler->read(stream))
        {
            return false;
        }
        assert(stream.i == stream.b.end());
    }

    ptrdiff_t pos = stream.i - stream.b.begin();
    if(pos < headerSize)
    {
        //
        // This situation is possible for small UDP packets.
        //
        throwIllegalMessageSizeException(__FILE__, __LINE__);
    }
    stream.i = stream.b.begin();
    const Byte* m;
    stream.readBlob(m, static_cast<Int>(sizeof(magic)));
    if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
    {
        throwBadMagicException(__FILE__, __LINE__, Ice::ByteSeq(&m[0], &m[0] + sizeof(magic)));
    }
    Byte pMajor;
    Byte pMinor;
    stream.read(pMajor);
    stream.read(pMinor);
    if(pMajor != protocolMajor
       || static_cast<unsigned char>(pMinor) > static_cast<unsigned char>(protocolMinor))
    {
        throwUnsupportedProtocolException(__FILE__, __LINE__, pMajor, pMinor, protocolMajor, protocolMinor);
    }
    Byte eMajor;
    Byte eMinor;
    stream.read(eMajor);
    stream.read(eMinor);
    if(eMajor != encodingMajor
       || static_cast<unsigned char>(eMinor) > static_cast<unsigned char>(encodingMinor))
    {
        throwUnsupportedEncodingException(__FILE__, __LINE__, eMajor, eMinor, encodingMajor, encodingMinor);
    }
    Byte messageType;
    stream.read(messageType);
    Byte compress;
    stream.read(compress);
    Int size;
    stream.read(size);
    if(size < headerSize)
    {
        throwIllegalMessageSizeException(__FILE__, __LINE__);
    }
    if(size > static_cast<Int>(_instance->messageSizeMax()))
    {
        throw MemoryLimitException(__FILE__, __LINE__);
    }
    if(size > static_cast<Int>(stream.b.size()))
    {
        stream.b.resize(size);
    }
    stream.i = stream.b.begin() + pos;

    if(stream.i != stream.b.end())
    {
        if(!handler->read(stream))
        {
            return false;
        }
        assert(stream.i == stream.b.end());
    }

    return true;
}

IceInternal::ThreadPool::EventHandlerThread::EventHandlerThread(const ThreadPoolPtr& pool) :
    _pool(pool)
{
}

void
IceInternal::ThreadPool::EventHandlerThread::run()
{
    bool promote;

    try
    {
        promote = _pool->run();
    }
    catch(const std::exception& ex)
    {
        Error out(_pool->_instance->initializationData().logger);
        out << "exception in `" << _pool->_prefix << "':\n" << ex.what();
        promote = true;
    }
    catch(...)
    {
        Error out(_pool->_instance->initializationData().logger);
        out << "unknown exception in `" << _pool->_prefix << "'";
        promote = true;
    }

    if(promote && _pool->_sizeMax > 1)
    {
        //
        // Promote a follower, but w/o modifying _inUse or creating
        // new threads.
        //
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*_pool.get());
            assert(!_pool->_promote);
            _pool->_promote = true;
            _pool->notify();
        }
    }

    _pool = 0; // Break cyclic dependency.
}
