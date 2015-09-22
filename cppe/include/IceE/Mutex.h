// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_MUTEX_H
#define ICEE_MUTEX_H

#include <IceE/Config.h>
#include <IceE/Lock.h>
#include <IceE/ThreadException.h>
#include <IceE/MutexProtocol.h>

namespace IceUtil
{

//
// Forward declaration for friend.
//
class Cond;

//
// Simple non-recursive Mutex implementation.
//
// Don't use noncopyable otherwise you end up with warnings like this:
//
// In file included from Connection.cpp:20:
// ../../include/IceE/Outgoing.h:88: warning: direct base
// `IceUtil::noncopyable' inaccessible in `IceInternal::Outgoing' due
// to ambiguity
//
class Mutex
{
public:

    //
    // Lock & TryLock typedefs.
    //
    typedef LockT<Mutex> Lock;
    typedef TryLockT<Mutex> TryLock;

    inline Mutex();
    explicit inline Mutex(MutexProtocol);
    ~Mutex();

    //
    // Note that lock/tryLock & unlock in general should not be used
    // directly. Instead use Lock & TryLock.
    //
 
    void lock() const;

    //
    // Returns true if the lock was acquired, and false otherwise.
    //
    bool tryLock() const;

    void unlock() const;

    //
    // Returns true if the mutex will unlock when calling unlock()
    // (false otherwise). For non-recursive mutexes, this will always
    // return true. 
    // This function is used by the Monitor implementation to know whether 
    // the Mutex has been locked for the first time, or unlocked for the 
    // last time (that is another thread is able to acquire the mutex).
    // Pre-condition: the mutex must be locked.
    //
    bool willUnlock() const;

private:

    inline void init(MutexProtocol);

    // noncopyable
    Mutex(const Mutex&);
    void operator=(const Mutex&);
    //
    // LockState and the lock/unlock variations are for use by the
    // Condition variable implementation.
    //
#ifdef _WIN32
    struct LockState
    {
    };
#else
    struct LockState
    {
        pthread_mutex_t* mutex;
    };
#endif

    void unlock(LockState&) const;
    void lock(LockState&) const;

    friend class Cond;

#ifdef _WIN32
    mutable CRITICAL_SECTION _mutex;
#ifdef _WIN32_WCE
    mutable int _recursionCount;
#endif
#else
    mutable pthread_mutex_t _mutex;
#endif
};

//
// For performance reasons the following functions are inlined.
//

inline
Mutex::Mutex()
#ifdef _WIN32_WCE
    : _recursionCount(0)
#endif
{
#ifdef _WIN32
    init(PrioNone);
#else
    init(getDefaultMutexProtocol());
#endif
}

inline
Mutex::Mutex(MutexProtocol protocol)
{
    init(protocol);
}

#ifdef _WIN32

inline void
Mutex::init(MutexProtocol)
{
    InitializeCriticalSection(&_mutex);
}

inline
Mutex::~Mutex()
{
    DeleteCriticalSection(&_mutex);
}

inline void
Mutex::lock() const
{
    EnterCriticalSection(&_mutex);
#ifdef _WIN32_WCE
    ++_recursionCount;
    assert(_recursionCount == 1);
#else
    assert(_mutex.RecursionCount == 1);
#endif
}

inline bool
Mutex::tryLock() const
{
    if(!TryEnterCriticalSection(&_mutex))
    {
        return false;
    }
#ifdef _WIN32_WCE
    if(_recursionCount > 0)
#else
    if(_mutex.RecursionCount > 1)
#endif
    {
        LeaveCriticalSection(&_mutex);
        throw ThreadLockedException(__FILE__, __LINE__);
    }

#ifdef _WIN32_WCE
    ++_recursionCount;
    assert(_recursionCount == 1);
#endif
    return true;
}

inline void
Mutex::unlock() const
{
#ifdef _WIN32_WCE
    --_recursionCount;
    assert(_recursionCount == 0);
#else
    assert(_mutex.RecursionCount == 1);
#endif
    LeaveCriticalSection(&_mutex);
}

inline void
Mutex::unlock(LockState&) const
{
    unlock();
}

inline void
Mutex::lock(LockState&) const
{
    lock();
}

#else

inline void
Mutex::init(MutexProtocol protocol)
{
#if defined(__linux) && !defined(__USE_UNIX98)
#   ifdef NDEBUG
    int rc = pthread_mutex_init(&_mutex, 0);
#   else
    const pthread_mutexattr_t attr = { PTHREAD_MUTEX_ERRORCHECK_NP };
    int rc = pthread_mutex_init(&_mutex, &attr);
#   endif
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#else // !defined(__linux) || defined(__USE_UNIX98)
    pthread_mutexattr_t attr;
    int rc = pthread_mutexattr_init(&attr);
    assert(rc == 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    //
    // Enable mutex error checking in debug builds
    //
#ifndef NDEBUG
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    assert(rc == 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif

    //
    // If system has support for priority inheritance we set the protocol
    // attribute of the mutex
    //
#if defined(_POSIX_THREAD_PRIO_INHERIT) && _POSIX_THREAD_PRIO_INHERIT > 0
    if(PrioInherit == protocol)
    {
        rc = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
        if(rc != 0)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
#endif

    rc = pthread_mutex_init(&_mutex, &attr);
    assert(rc == 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    rc = pthread_mutexattr_destroy(&attr);
    assert(rc == 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif
}

inline
Mutex::~Mutex()
{
    int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
}

inline void
Mutex::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
        if(rc == EDEADLK)
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        else
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
}

inline bool
Mutex::tryLock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0 && rc != EBUSY)
    {
        if(rc == EDEADLK)
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        else
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    return (rc == 0);
}

inline void
Mutex::unlock() const
{
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

inline void
Mutex::unlock(LockState& state) const
{
    state.mutex = &_mutex;
}

inline void
Mutex::lock(LockState&) const
{
}

#endif    

inline bool
Mutex::willUnlock() const
{
    return true;
}

} // End namespace IceUtil

#endif
