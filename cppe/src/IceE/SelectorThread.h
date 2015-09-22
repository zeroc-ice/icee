// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_SELECTOR_THREAD_H
#define ICEE_SELECTOR_THREAD_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>
#include <IceE/Mutex.h>
#include <IceE/Thread.h>
#include <IceE/Timer.h>

#include <IceE/Config.h>
#include <IceE/SelectorThreadF.h>
#include <IceE/SocketReadyCallback.h>
#include <IceE/InstanceF.h>
#include <IceE/Selector.h>

#include <deque>

namespace IceInternal
{

class SelectorThread : public IceUtil::Shared, IceUtil::Mutex
{
public:

    SelectorThread(const InstancePtr&);
    virtual ~SelectorThread();

    void destroy();

    void incFdsInUse();
    void decFdsInUse();

    void _register(SOCKET, const SocketReadyCallbackPtr&, SocketStatus status, int timeout);
    void unregister(const SocketReadyCallbackPtr&);
    void finish(const SocketReadyCallbackPtr&);

    void joinWithThread();

private:

    void run();

    class HelperThread : public IceUtil::Thread
    {
    public:
        
        HelperThread(const SelectorThreadPtr&);
        virtual void run();

    private:

        SelectorThreadPtr _selectorThread;
    };
    friend class HelperThread;

    InstancePtr _instance;
    bool _destroyed;

    Selector<SocketReadyCallback> _selector;

    std::deque<SocketReadyCallbackPtr> _finished;

    IceUtil::ThreadPtr _thread;
    IceUtil::TimerPtr _timer;
};

}

#endif
