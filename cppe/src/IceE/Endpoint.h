// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ENDPOINT_H
#define ICEE_ENDPOINT_H

#include <IceE/EndpointF.h>
#include <IceE/ConnectorF.h>
#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>

#ifndef ICEE_PURE_CLIENT
#   include <IceE/AcceptorF.h>
#endif

#include <IceE/Shared.h>
#include <IceE/Exception.h>
#include <IceE/Thread.h>
#include <IceE/Monitor.h>

#include <list>

#ifdef _WIN32
#  include <winsock2.h>
#else
#  include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class BasicStream;

#ifdef ICEE_HAS_AMI
class Endpoint_connectors : public virtual IceUtil::Shared
{
public:

    virtual ~Endpoint_connectors() { }

    virtual void connectors(const std::vector<ConnectorPtr>&) = 0;
    virtual void exception(const Ice::LocalException&) = 0;
};
typedef IceUtil::Handle<Endpoint_connectors> Endpoint_connectorsPtr;
#endif

class Endpoint : public IceUtil::Shared
{
public:

    //
    // Marshal the endpoint.
    //
    virtual void streamWrite(BasicStream*) const = 0;

    //
    // Convert the endpoint to its string form.
    //
    virtual std::string toString() const = 0;

    //
    // Return the endpoint type.
    //
    virtual Ice::Short type() const = 0;

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    virtual Ice::Int timeout() const = 0;

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    virtual EndpointPtr timeout(Ice::Int) const = 0;
    
    //
    // Return true if the endpoint is datagram-based.
    //
    virtual bool datagram() const = 0;

    //
    // Return true if the endpoint is secure.
    //
    virtual bool secure() const = 0;

    //
    // Return true if the endpoint type is unknown.
    //
    virtual bool unknown() const = 0;

    //
    // Return connectors for this endpoint, or empty vector if no
    // connector is available.
    //
#ifndef ICEE_HAS_AMI
    virtual std::vector<ConnectorPtr> connectors() const = 0;
#else
    virtual void connectors_async(const Endpoint_connectorsPtr&) const = 0;
#endif

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
#ifndef ICEE_PURE_CLIENT
    virtual AcceptorPtr acceptor(EndpointPtr&) const = 0;

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if endpoint was configured with no host set.
    //
    virtual std::vector<EndpointPtr> expand() const = 0;
#endif

    //
    // Compare endpoints for sorting purposes.
    //
    virtual bool operator==(const Endpoint&) const = 0;
    virtual bool operator<(const Endpoint&) const = 0;

protected:

#ifdef ICEE_HAS_AMI
    virtual std::vector<ConnectorPtr> connectors(const std::vector<struct sockaddr_in>&) const;
    friend class EndpointHostResolver;
#endif
};

#ifdef ICEE_HAS_AMI
class EndpointHostResolver : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{   
public:
    
    EndpointHostResolver(const InstancePtr&);

    void resolve(const std::string&, int, const EndpointPtr&, const Endpoint_connectorsPtr&);
    void destroy();
    
    virtual void run();

private:

    struct ResolveEntry
    {
        std::string host;
        int port;
        EndpointPtr endpoint;
        Endpoint_connectorsPtr callback;
    };

    const InstancePtr _instance;
    bool _destroyed;
    std::list<ResolveEntry> _queue;
};
#endif

}

#endif
