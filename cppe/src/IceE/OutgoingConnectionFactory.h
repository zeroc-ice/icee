// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_CONNECTION_FACTORY_H
#define ICEE_OUTGOING_CONNECTION_FACTORY_H

#include <IceE/OutgoingConnectionFactoryF.h>
#include <IceE/ConnectionI.h>
#include <IceE/ConnectRequestHandler.h>
#include <IceE/InstanceF.h>
#if defined(ICEE_HAS_ROUTER) && !defined(ICEE_PURE_CLIENT)
#   include <IceE/ObjectAdapterF.h>
#   include <IceE/RouterInfoF.h>
#endif
#include <IceE/Endpoint.h>
#include <IceE/ConnectorF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <set>

namespace IceInternal
{

class OutgoingConnectionFactory : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void destroy();

    void waitUntilFinished();

#ifndef ICEE_HAS_AMI
    Ice::ConnectionIPtr create(const std::vector<EndpointPtr>&);
#else
    void create(const std::vector<EndpointPtr>&, const ConnectRequestHandlerPtr&);
#endif
#if defined(ICEE_HAS_ROUTER) && !defined(ICEE_PURE_CLIENT)
    void setRouterInfo(const RouterInfoPtr&);
    void removeAdapter(const Ice::ObjectAdapterPtr&);
#endif
#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

private:

    OutgoingConnectionFactory(const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    friend class Instance;

    struct ConnectorInfo
    {
        ConnectorInfo(const ConnectorPtr& c, const EndpointPtr& e) : connector(c), endpoint(e)
        {
        }

        bool operator<(const ConnectorInfo& other) const;

        ConnectorPtr connector;
        EndpointPtr endpoint;
    };

#ifdef ICEE_HAS_AMI
    class ConnectCallback : public Ice::ConnectionI::StartCallback, public IceInternal::Endpoint_connectors
    {
    public:

        ConnectCallback(const OutgoingConnectionFactoryPtr&, const std::vector<EndpointPtr>&, 
                        const ConnectRequestHandlerPtr&);

        virtual void connectionStartCompleted(const Ice::ConnectionIPtr&);
        virtual void connectionStartFailed(const Ice::ConnectionIPtr&, const Ice::LocalException&);

        virtual void connectors(const std::vector<ConnectorPtr>&);
        virtual void exception(const Ice::LocalException&);

        void getConnectors();
        void nextEndpoint();

        void getConnection();
        void nextConnector();

        bool operator<(const ConnectCallback&) const;
        
    private:

        const OutgoingConnectionFactoryPtr _factory;
        const std::vector<EndpointPtr> _endpoints;
        const ConnectRequestHandlerPtr _handler;
        std::vector<EndpointPtr>::const_iterator _endpointsIter;
        std::vector<ConnectorInfo> _connectors;
        std::vector<ConnectorInfo>::const_iterator _iter;
    };
    typedef IceUtil::Handle<ConnectCallback> ConnectCallbackPtr;
    friend class ConnectCallback;
#endif

    std::vector<EndpointPtr> applyOverrides(const std::vector<EndpointPtr>&);
    Ice::ConnectionIPtr findConnection(const std::vector<EndpointPtr>&);
    void incPendingConnectCount();
    void decPendingConnectCount();
#ifndef ICEE_HAS_AMI
    Ice::ConnectionIPtr getConnection(const std::vector<ConnectorInfo>&);
#else
    Ice::ConnectionIPtr getConnection(const std::vector<ConnectorInfo>&, const ConnectCallbackPtr&);
#endif
    void finishGetConnection(const std::vector<ConnectorInfo>&, const Ice::ConnectionIPtr&);
    Ice::ConnectionIPtr findConnection(const std::vector<ConnectorInfo>&);
    Ice::ConnectionIPtr createConnection(const TransceiverPtr&, const ConnectorInfo&);

    void handleException(const Ice::LocalException&, bool);
    void handleException(const Ice::LocalException&, const ConnectorInfo&, const Ice::ConnectionIPtr&, bool);

    const InstancePtr _instance;
    bool _destroyed;

    std::multimap<ConnectorInfo, Ice::ConnectionIPtr> _connections;
#ifndef ICEE_HAS_AMI
    std::set<ConnectorInfo > _pending;
#else
    std::map<ConnectorInfo, std::set<ConnectCallbackPtr> > _pending;
#endif

    std::multimap<EndpointPtr, Ice::ConnectionIPtr> _connectionsByEndpoint;
    int _pendingConnectCount;
};

}

#endif
