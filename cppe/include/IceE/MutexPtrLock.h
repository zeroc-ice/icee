// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_MUTEXPTRLOCK_H
#define ICEE_MUTEXPTRLOCK_H

#include <IceE/Config.h>
#include <IceE/Mutex.h>

namespace IceUtilInternal
{

class MutexPtrLock
{
public:
    
    MutexPtrLock(const IceUtil::Mutex* mutex) : _mutex(mutex)
    {
        if(_mutex)
        {
            _mutex->lock();
        }
    }

    ~MutexPtrLock()
    {
        if(_mutex)
        {
            _mutex->unlock();
        }
    }
   
private:
    
    // Not implemented; prevents accidental use.
    //
    MutexPtrLock(const MutexPtrLock&);
    MutexPtrLock& operator=(const MutexPtrLock&);

    const IceUtil::Mutex* _mutex;
};

} // End namespace IceUtilInternal

#endif
