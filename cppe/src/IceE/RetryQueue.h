// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_RETRY_QUEUE_H
#define ICEE_RETRY_QUEUE_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Timer.h>
#include <IceE/RetryQueueF.h>
#include <IceE/OutgoingAsyncF.h>
#include <IceE/InstanceF.h>

#ifdef ICEE_HAS_AMI

namespace IceInternal
{

class RetryTask : public IceUtil::TimerTask
{
public:
    
    RetryTask(const RetryQueuePtr&, const OutgoingAsyncPtr&);
    
    virtual void runTimerTask();
    void destroy();
    
    bool operator<(const RetryTask&) const;
    
private:
    
    const RetryQueuePtr _queue;
    const OutgoingAsyncPtr _outAsync;
};
typedef IceUtil::Handle<RetryTask> RetryTaskPtr;

class RetryQueue : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    RetryQueue(const InstancePtr&);
    
    void add(const OutgoingAsyncPtr&, int);
    void destroy();

private:

    bool remove(const RetryTaskPtr&);
    friend class RetryTask;

    const InstancePtr _instance;
    std::set<RetryTaskPtr> _requests;
};

}

#endif

#endif
