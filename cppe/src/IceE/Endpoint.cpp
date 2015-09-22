// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Endpoint.h>
#include <IceE/Network.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(Endpoint* p) { return p; }

#ifdef ICEE_HAS_AMI
IceUtil::Shared* IceInternal::upCast(EndpointHostResolver* p) { return p; }

vector<ConnectorPtr>
IceInternal::Endpoint::connectors(const vector<struct sockaddr_in>&) const
{
    //
    // This method must be extended by endpoints which use the EndpointHostResolver to create
    // connectors from IP addresses.
    //
    assert(false);
    return vector<ConnectorPtr>();
}

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    _instance(instance),
    _destroyed(false)
{
    __setNoDelete(true);
    if(_instance->initializationData().properties->getProperty("Ice.ThreadPriority") != "")
    {
        start(0, _instance->initializationData().properties->getPropertyAsInt("Ice.ThreadPriority"));
    }
    else
    {
        start();
    }
    __setNoDelete(false);
}

void
IceInternal::EndpointHostResolver::resolve(const string& host, int port, const EndpointPtr& endpoint,
                                           const Endpoint_connectorsPtr& callback)
{ 
    //
    // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
    // entry and the thread will take care of getting the endpoint addresses.
    //
    try
    {
        vector<struct sockaddr_in> addrs = getAddresses(host, port, false, false);
        if(!addrs.empty())
        {
            callback->connectors(endpoint->connectors(addrs));
            return;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        callback->exception(ex);
        return;
    }

    Lock sync(*this);
    assert(!_destroyed);

    ResolveEntry entry;
    entry.host = host;
    entry.port = port;
    entry.endpoint = endpoint;
    entry.callback = callback;
    _queue.push_back(entry);
    notify();
}

void
IceInternal::EndpointHostResolver::destroy()
{
    Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    notify();
}

void
IceInternal::EndpointHostResolver::run()
{
    while(true)
    {
        ResolveEntry resolve;

        {
            Lock sync(*this);

            while(!_destroyed && _queue.empty())
            {
                wait();
            }

            if(_destroyed)
            {
                break;
            }

            resolve = _queue.front();
            _queue.pop_front();
        }

        try
        {
            resolve.callback->connectors(
                resolve.endpoint->connectors(
                    getAddresses(resolve.host, resolve.port, false, true)));
        }
        catch(const Ice::LocalException& ex)
        {
            resolve.callback->exception(ex);
        }
    }

    for(list<ResolveEntry>::const_iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        p->callback->exception(Ice::CommunicatorDestroyedException(__FILE__, __LINE__));
    }
    _queue.clear();
}

#endif
