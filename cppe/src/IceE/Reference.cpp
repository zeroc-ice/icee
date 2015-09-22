// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Reference.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/Endpoint.h>
#include <IceE/BasicStream.h>

#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterInfo.h>
#   include <IceE/Router.h>
#endif

#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorInfo.h>
#   include <IceE/Locator.h>
#endif

#include <IceE/Connection.h>
#include <IceE/ConnectRequestHandler.h>
#include <IceE/Functional.h>
#include <IceE/OutgoingConnectionFactory.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/StringUtil.h>
#include <IceE/Random.h>

#include <functional>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(IceInternal::Reference* p) { return p; }
namespace
{
struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
	return IceUtilInternal::random(static_cast<int>(d));
    }
};
}

CommunicatorPtr
IceInternal::Reference::getCommunicator() const
{
    return _communicator;
}

ReferencePtr
IceInternal::Reference::changeContext(const Context& newContext) const
{
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_context = newContext;
    return r;
}

ReferencePtr
IceInternal::Reference::changeMode(ReferenceMode newMode) const
{
    if(newMode == _mode)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_mode = newMode;
    return r;
}

ReferencePtr
IceInternal::Reference::changeSecure(bool newSecure) const
{
    if(newSecure == _secure)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_secure = newSecure;
    return r;
}

ReferencePtr
IceInternal::Reference::changeIdentity(const Identity& newIdentity) const
{
    if(newIdentity == _identity)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_identity = newIdentity;
    return r;
}

ReferencePtr
IceInternal::Reference::changeFacet(const string& newFacet) const
{
    if(newFacet == _facet)
    {
        return ReferencePtr(const_cast<Reference*>(this));
    }
    ReferencePtr r = _instance->referenceFactory()->copy(this);
    r->_facet = newFacet;
    return r;
}

Int
Reference::hash() const
{
    IceUtil::Mutex::Lock sync(_hashMutex);
    if(!_hashInitialized)
    {
        hashInit(); // Initialize _hashValue
    }
    return _hashValue;
}

void
IceInternal::Reference::streamWrite(BasicStream* s) const
{
    //
    // Don't write the identity here. Operations calling streamWrite
    // write the identity.
    //

    //
    // For compatibility with the old FacetPath.
    //
    if(_facet.empty())
    {
        s->write(static_cast<string*>(0), static_cast<string*>(0));
    }
    else
    {
        s->write(&_facet, &_facet + 1);
    }

    s->write(static_cast<Byte>(_mode));

    s->write(_secure);

    // Derived class writes the remainder of the reference.
}

string
IceInternal::Reference::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    string s;

    //
    // If the encoded identity string contains characters which
    // the reference parser uses as separators, then we enclose
    // the identity string in quotes.
    //
    string id = _instance->identityToString(_identity);
    if(id.find_first_of(" :@") != string::npos)
    {
        s += "\"";
        s += id;
        s += "\"";
    }
    else
    {
        s += id;
    }

    if(!_facet.empty())
    {
        s += " -f ";

        //
        // If the encoded facet string contains characters which
        // the reference parser uses as separators, then we enclose
        // the facet string in quotes.
        //
        string fs = _facet;
#ifdef ICEE_HAS_WSTRING
        if(_instance->initializationData().stringConverter)
        {
            UTF8BufferI buffer;
            Byte* last =
                _instance->initializationData().stringConverter->toUTF8(fs.data(), fs.data() + fs.size(), buffer);
            fs = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
#endif
        fs = IceUtilInternal::escapeString(fs, "");
        if(fs.find_first_of(" :@") != string::npos)
        {
            s += "\"";
            s += fs;
            s += "\"";
        }
        else
        {
            s += fs;
        }
    }

    switch(_mode)
    {
        case ReferenceModeTwoway:
        {
            s += " -t";
            break;
        }

        case ReferenceModeOneway:
        {
            s += " -o";
            break;
        }

        case ReferenceModeBatchOneway:
        {
            s += " -O";
            break;
        }

        case ReferenceModeDatagram:
        {
            s += " -d";
            break;
        }

        case ReferenceModeBatchDatagram:
        {
            s += " -D";
            break;
        }
    }

    if(_secure)
    {
        s += " -s";
    }

    return s;

    // Derived class writes the remainder of the string.
}

bool
IceInternal::Reference::operator==(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    if(_mode != r._mode)
    {
        return false;
    }

    if(_secure != r._secure)
    {
        return false;
    }

    if(_identity != r._identity)
    {
        return false;
    }

    if(_context != r._context)
    {
        return false;
    }

    if(_facet != r._facet)
    {
        return false;
    }

    return true;
}

bool
IceInternal::Reference::operator!=(const Reference& r) const
{
    return !operator==(r);
}

bool
IceInternal::Reference::operator<(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //

    if(_mode < r._mode)
    {
        return true;
    }
    else if(r._mode < _mode)
    {
        return false;
    }

    if(_identity < r._identity)
    {
        return true;
    }
    else if(r._identity < _identity)
    {
        return false;
    }

    if(_context < r._context)
    {
        return true;
    }
    else if(r._context < _context)
    {
        return false;
    }

    if(_facet < r._facet)
    {
        return true;
    }
    else if(r._facet < _facet)
    {
        return false;
    }

    if(!_secure && r._secure)
    {
        return true;
    }
    else if(r._secure < _secure)
    {
        return false;
    }

    return false;
}

IceInternal::Reference::Reference(const InstancePtr& instance,
                                  const CommunicatorPtr& communicator,
                                  const Identity& id,
                                  const Context& context,
                                  const string& facet,
                                  ReferenceMode mode,
                                  bool secure) :
    _hashInitialized(false),
    _instance(instance),
    _communicator(communicator),
    _mode(mode),
    _secure(secure),
    _identity(id),
    _context(context),
    _facet(facet)
{
}

IceInternal::Reference::Reference(const Reference& r) :
    _hashInitialized(false),
    _instance(r._instance),
    _communicator(r._communicator),
    _mode(r._mode),
    _secure(r._secure),
    _identity(r._identity),
    _context(r._context),
    _facet(r._facet)
{
}

void
Reference::hashInit() const
{
    string::const_iterator p;
    Context::const_iterator q;

    Int h = static_cast<Int>(_mode);

    for(p = _identity.name.begin(); p != _identity.name.end(); ++p)
    {
        h = 5 * h + *p;
    }

    for(p = _identity.category.begin(); p != _identity.category.end(); ++p)
    {
        h = 5 * h + *p;
    }

    for(q = _context.begin(); q != _context.end(); ++q)
    {
        for(p = q->first.begin(); p != q->first.end(); ++p)
        {
            h = 5 * h + *p;
        }
        for(p = q->second.begin(); p != q->second.end(); ++p)
        {
            h = 5 * h + *p;
        }
    }

    for(p = _facet.begin(); p != _facet.end(); ++p)
    {
        h = 5 * h + *p;
    }

    h = 5 * h + static_cast<Int>(_secure);

    _hashValue = h;
    _hashInitialized = true;
}

IceUtil::Shared* IceInternal::upCast(IceInternal::FixedReference* p) { return p; }

IceInternal::FixedReference::FixedReference(const InstancePtr& instance,
                                            const CommunicatorPtr& communicator,
                                            const Identity& id,
                                            const Context& context,
                                            const string& facet,
                                            ReferenceMode mode,
                                            bool secure,
                                            const ConnectionIPtr& fixedConnection) :
    Reference(instance, communicator, id, context, facet, mode, secure),
    _fixedConnection(fixedConnection)
{
}

vector<EndpointPtr>
IceInternal::FixedReference::getEndpoints() const
{
    return vector<EndpointPtr>();
}

string
IceInternal::FixedReference::getAdapterId() const
{
    return string();
}

ReferencePtr
IceInternal::FixedReference::changeAdapterId(const std::string&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

#ifdef ICEE_HAS_LOCATOR

ReferencePtr
IceInternal::FixedReference::changeLocator(const LocatorPrx&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

#endif

#ifdef ICEE_HAS_ROUTER

ReferencePtr
IceInternal::FixedReference::changeRouter(const RouterPrx&) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

#endif

ReferencePtr
IceInternal::FixedReference::changeTimeout(int) const
{
    throw FixedProxyException(__FILE__, __LINE__);
    return 0; // Keep the compiler happy.
}

bool
IceInternal::FixedReference::isIndirect() const
{
    return false;
}

bool
IceInternal::FixedReference::isWellKnown() const
{
    return false;
}

void
IceInternal::FixedReference::streamWrite(BasicStream* s) const
{
    throw FixedProxyException(__FILE__, __LINE__);
}

string
IceInternal::FixedReference::toString() const
{
    throw FixedProxyException(__FILE__, __LINE__);

    assert(false);   // Cannot be reached.
    return string(); // To keep the compiler from complaining.
}

#ifndef ICEE_HAS_AMI
Ice::ConnectionIPtr
IceInternal::FixedReference::getConnection() const
{
    switch(getMode())
    {
        case ReferenceModeTwoway:
        case ReferenceModeOneway:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchOneway:
#endif
        {
            if(_fixedConnection->endpoint()->datagram())
            {
                throw NoEndpointException(__FILE__, __LINE__, "");
            }
            break;
        }

        case ReferenceModeDatagram:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
#endif
        {
            if(!_fixedConnection->endpoint()->datagram())
            {
                throw NoEndpointException(__FILE__, __LINE__, "");
            }
            break;
        }

#ifndef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
        case ReferenceModeBatchOneway:
        {
            throw FeatureNotSupportedException(__FILE__, __LINE__, "batch proxy mode");
        }
#endif
    }

    //
    // If a secure connection is requested, check if the connection is secure.
    //
    bool secure = getSecure();
    if(secure && !_fixedConnection->endpoint()->secure())
    {
        throw NoEndpointException(__FILE__, __LINE__, "");
    }

    _fixedConnection->throwException(); // Throw in case our connection is already destroyed.
    return _fixedConnection;
}

#else

void
IceInternal::FixedReference::getConnection(const ConnectRequestHandlerPtr& handler) const
{
    switch(getMode())
    {
        case ReferenceModeTwoway:
        case ReferenceModeOneway:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchOneway:
#endif
        {
            if(_fixedConnection->endpoint()->datagram())
            {
                handler->setException(NoEndpointException(__FILE__, __LINE__, ""));
                return;
            }
            break;
        }

        case ReferenceModeDatagram:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
#endif
        {
            if(!_fixedConnection->endpoint()->datagram())
            {
                handler->setException(NoEndpointException(__FILE__, __LINE__, ""));
                return;
            }
            break;
        }

#ifndef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
        case ReferenceModeBatchOneway:
        {
            handler->setException(FeatureNotSupportedException(__FILE__, __LINE__, "batch proxy mode"));
            return;
        }
#endif
    }

    //
    // If a secure connection is requested, check if the connection is secure.
    //
    bool secure = getSecure();
    if(secure && !_fixedConnection->endpoint()->secure())
    {
        handler->setException(NoEndpointException(__FILE__, __LINE__, ""));
        return;
    }

    try
    {
        _fixedConnection->throwException(); // Throw in case our connection is already destroyed.
    }
    catch(const Ice::LocalException& ex)
    {
        handler->setException(ex);
        return;
    }

    handler->setConnection(_fixedConnection);
}
#endif

bool
IceInternal::FixedReference::operator==(const Reference& r) const
{
    if(this == &r)
    {
        return true;
    }
    const FixedReference* rhs = dynamic_cast<const FixedReference*>(&r);
    if(!rhs || !Reference::operator==(r))
    {
        return false;
    }
    return _fixedConnection == rhs->_fixedConnection;
}

bool
IceInternal::FixedReference::operator!=(const Reference& r) const
{
    return !operator==(r);
}

bool
IceInternal::FixedReference::operator<(const Reference& r) const
{
    if(this == &r)
    {
        return false;
    }
    if(Reference::operator<(r))
    {
        return true;
    }
    if(!Reference::operator==(r))
    {
        return false;
    }

    const FixedReference* rhs = dynamic_cast<const FixedReference*>(&r);
    if(!rhs)
    {
        assert(dynamic_cast<const RoutableReference*>(&r));
        return false; // As a rule, routable references are superior to fixed references.
    }
    return _fixedConnection < rhs->_fixedConnection;
}

ReferencePtr
IceInternal::FixedReference::clone() const
{
    return new FixedReference(*this);
}

IceInternal::FixedReference::FixedReference(const FixedReference& r) :
    Reference(r),
    _fixedConnection(r._fixedConnection)
{
}

IceUtil::Shared* IceInternal::upCast(IceInternal::RoutableReference* p) { return p; }

IceInternal::RoutableReference::RoutableReference(const InstancePtr& instance,
                                                  const CommunicatorPtr& communicator,
                                                  const Identity& id,
                                                  const Context& context,
                                                  const string& facet,
                                                  ReferenceMode mode,
                                                  bool secure,
                                                  const vector<EndpointPtr>& endpoints,
                                                  const string& adapterId
#ifdef ICEE_HAS_LOCATOR
                                                  , const LocatorInfoPtr& locatorInfo
#endif
#ifdef ICEE_HAS_ROUTER
                                                  , const RouterInfoPtr& routerInfo
#endif
                                                  ) :
    Reference(instance, communicator, id, context, facet, mode, secure),
    _endpoints(endpoints),
    _adapterId(adapterId),
#ifdef ICEE_HAS_LOCATOR
    _locatorInfo(locatorInfo),
#endif
#ifdef ICEE_HAS_ROUTER
    _routerInfo(routerInfo),
#endif
    _overrideTimeout(false),
    _timeout(-1)
{
    assert(_adapterId.empty() || _endpoints.empty());
}

vector<EndpointPtr>
IceInternal::RoutableReference::getEndpoints() const
{
    return _endpoints;
}

string
IceInternal::RoutableReference::getAdapterId() const
{
    return _adapterId;
}

#ifdef ICEE_HAS_LOCATOR

LocatorInfoPtr
IceInternal::RoutableReference::getLocatorInfo() const
{
    return _locatorInfo;
}

#endif

#ifdef ICEE_HAS_ROUTER

RouterInfoPtr
IceInternal::RoutableReference::getRouterInfo() const
{
    return _routerInfo;
}

#endif

ReferencePtr
IceInternal::RoutableReference::changeAdapterId(const string& newAdapterId) const
{
    if(newAdapterId == _adapterId)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_adapterId = newAdapterId;
    r->_endpoints.clear();
    return r;
}

#ifdef ICEE_HAS_LOCATOR

ReferencePtr
IceInternal::RoutableReference::changeLocator(const LocatorPrx& newLocator) const
{
    LocatorInfoPtr newLocatorInfo = getInstance()->locatorManager()->get(newLocator);
    if(newLocatorInfo == _locatorInfo)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_locatorInfo = newLocatorInfo;
    return r;
}

#endif

#ifdef ICEE_HAS_ROUTER

ReferencePtr
IceInternal::RoutableReference::changeRouter(const RouterPrx& newRouter) const
{
    RouterInfoPtr newRouterInfo = getInstance()->routerManager()->get(newRouter);
    if(newRouterInfo == _routerInfo)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_routerInfo = newRouterInfo;
    return r;
}

#endif


ReferencePtr
IceInternal::RoutableReference::changeTimeout(int newTimeout) const
{
    if(_overrideTimeout && newTimeout == _timeout)
    {
        return RoutableReferencePtr(const_cast<RoutableReference*>(this));
    }
    RoutableReferencePtr r = RoutableReferencePtr::dynamicCast(getInstance()->referenceFactory()->copy(this));
    r->_timeout = newTimeout;
    r->_overrideTimeout = true;
    if(!_endpoints.empty()) // Also override the timeout on the endpoints.
    {
        vector<EndpointPtr> newEndpoints;
        for(vector<EndpointPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            newEndpoints.push_back((*p)->timeout(newTimeout));
        }
        r->_endpoints = newEndpoints;
    }
    return r;
}

bool
IceInternal::RoutableReference::isIndirect() const
{
    return _endpoints.empty();
}

bool
IceInternal::RoutableReference::isWellKnown() const
{
    return _endpoints.empty() && _adapterId.empty();
}

void
IceInternal::RoutableReference::streamWrite(BasicStream* s) const
{
    Reference::streamWrite(s);

    Int sz = static_cast<Int>(_endpoints.size());
    s->writeSize(sz);
    if(sz)
    {
        assert(_adapterId.empty());
        for(vector<EndpointPtr>::const_iterator p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            (*p)->streamWrite(s);
        }
    }
    else
    {
        s->write(_adapterId);
    }
}

string
IceInternal::RoutableReference::toString() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    string result = Reference::toString();

    if(!_endpoints.empty())
    {
        vector<EndpointPtr>::const_iterator p;
        for(p = _endpoints.begin(); p != _endpoints.end(); ++p)
        {
            string endp = (*p)->toString();
            if(!endp.empty())
            {
                result.append(":");
                result.append(endp);
            }
        }
    }
    else if(!_adapterId.empty())
    {
        result.append(" @ ");

        //
        // If the encoded adapter id string contains characters which the
        // reference parser uses as separators, then we enclose the
        // adapter id string in quotes.
        //
        string a = _adapterId;
#ifdef ICEE_HAS_WSTRING
        StringConverterPtr stringConverter = getInstance()->initializationData().stringConverter;
        if(stringConverter)
        {
            UTF8BufferI buffer;
            Byte* last = stringConverter->toUTF8(a.data(), a.data() + a.size(), buffer);
            a = string(reinterpret_cast<const char*>(buffer.getBuffer()), last - buffer.getBuffer());
        }
#endif
        a = IceUtilInternal::escapeString(a, "");
        if(a.find_first_of(" ") != string::npos)
        {
            result.append("\"");
            result.append(a);
            result.append("\"");
        }
        else
        {
            result.append(_adapterId);
        }
    }
    else
    {
        return result;
    }
    return result;
}

int
IceInternal::RoutableReference::hash() const
{
    IceUtil::Mutex::Lock sync(_hashMutex);
    if(!_hashInitialized)
    {
        hashInit(); // Initializes _hashValue.

        // Add hash of adapter ID to base hash.
        for(string::const_iterator p = _adapterId.begin(); p != _adapterId.end(); ++p)
        {
            _hashValue = 5 * _hashValue + *p;
        }
    }
    return _hashValue;
}

bool
IceInternal::RoutableReference::operator==(const Reference& r) const
{
    //
    // Note: if(this == &r) test is performed by each non-abstract derived class.
    //
    if(this == &r)
    {
        return true;
    }

    const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
    if(!rhs || !Reference::operator==(r))
    {
        return false;
    }
    if((_overrideTimeout != rhs->_overrideTimeout) || (_overrideTimeout && _timeout != rhs->_timeout))
    {
        return false;
    }
#ifdef ICEE_HAS_ROUTER
    if(_routerInfo != rhs->_routerInfo)
    {
        return false;
    }
#endif
#ifdef ICEE_HAS_LOCATOR
    if(_locatorInfo != rhs->_locatorInfo)
    {
        return false;
    }
#endif
    if(_endpoints != rhs->_endpoints)
    {
        return false;
    }
    if(_adapterId != rhs->_adapterId)
    {
        return false;
    }
    return true;
}

bool
IceInternal::RoutableReference::operator!=(const Reference& r) const
{
    return !operator==(r);
}

bool
IceInternal::RoutableReference::operator<(const Reference& r) const
{
    if(this == &r)
    {
        return false;
    }

    if(Reference::operator<(r))
    {
        return true;
    }
    else if(!Reference::operator==(r))
    {
        return false;
    }

    const RoutableReference* rhs = dynamic_cast<const RoutableReference*>(&r);
    if(!rhs)
    {
        assert(dynamic_cast<const FixedReference*>(&r));
        return true; // As a rule, routable references are superior to fixed references.
    }

    if(!_overrideTimeout && rhs->_overrideTimeout)
    {
        return true;
    }
    else if(rhs->_overrideTimeout < _overrideTimeout)
    {
        return false;
    }
    else if(_overrideTimeout)
    {
        if(_timeout < rhs->_timeout)
        {
            return true;
        }
        else if(rhs->_timeout < _timeout)
        {
            return false;
        }
    }
#ifdef ICEE_HAS_ROUTER
    if(_routerInfo < rhs->_routerInfo)
    {
        return true;
    }
    else if(rhs->_routerInfo < _routerInfo)
    {
        return false;
    }
#endif
#ifdef ICEE_HAS_LOCATOR
    if(_locatorInfo < rhs->_locatorInfo)
    {
        return true;
    }
    else if(rhs->_locatorInfo < _locatorInfo)
    {
        return false;
    }
#endif
    if(_adapterId < rhs->_adapterId)
    {
        return true;
    }
    else if(rhs->_adapterId < _adapterId)
    {
        return false;
    }
    if(_endpoints < rhs->_endpoints)
    {
        return true;
    }
    else if(rhs->_endpoints < _endpoints)
    {
        return false;
    }
    return false;
}

ReferencePtr
IceInternal::RoutableReference::clone() const
{
    return new RoutableReference(*this);
}

#ifndef ICEE_HAS_AMI

Ice::ConnectionIPtr
IceInternal::RoutableReference::getConnection() const
{
#ifdef ICEE_HAS_ROUTER
    if(_routerInfo)
    {
	//
	// If we route, we send everything to the router's client
	// proxy endpoints.
	//
	vector<EndpointPtr> endpts = _routerInfo->getClientEndpoints();
	if(!endpts.empty())
	{
	    applyOverrides(endpts);
            return createConnection(endpts);
	}
    }
#endif

    if(!_endpoints.empty())
    {
        return createConnection(_endpoints);
    }

#ifdef ICEE_HAS_LOCATOR
    while(true)
    {
	bool cached = false;
	vector<EndpointPtr> endpts;
        if(_locatorInfo)
	{
	    endpts = _locatorInfo->getEndpoints(const_cast<RoutableReference*>(this), cached);
	    applyOverrides(endpts);
	}

	if(endpts.empty())
	{
	    throw Ice::NoEndpointException(__FILE__, __LINE__, toString());
	}

	try
	{
	    return createConnection(endpts);
	}
	catch(const NoEndpointException&)
	{
	    throw; // No need to retry if there's no endpoints.
	}
	catch(const LocalException& ex)
	{
	    assert(_locatorInfo);
	    _locatorInfo->clearCache(const_cast<RoutableReference*>(this));

	    if(cached)
	    {
		// COMPILERFIX: Braces needed to prevent BCB from causing TraceLevels refCount from
		//		being decremented twice when loop continues.
		{
		    TraceLevelsPtr traceLevels = getInstance()->traceLevels();
		    if(traceLevels->retry >= 2)
		    {
			Trace out(getInstance()->initializationData().logger, traceLevels->retryCat);
			out << "connection to cached endpoints failed\n"
			    << "removing endpoints from cache and trying one more time\n" << ex.toString();
		    }
		}
		continue;
	    }
	    throw;
	}
    }

    assert(false);
    return 0;
#else
    throw Ice::NoEndpointException(__FILE__, __LINE__, toString());
#endif
}

Ice::ConnectionIPtr
IceInternal::RoutableReference::createConnection(const vector<EndpointPtr>& endpoints) const
{
    vector<EndpointPtr> endpts = filterEndpoints(endpoints);
    if(endpts.empty())
    {
        throw Ice::NoEndpointException(__FILE__, __LINE__, toString());
    }

    return getInstance()->outgoingConnectionFactory()->create(endpts);
}

#else

void
IceInternal::RoutableReference::getConnection(const ConnectRequestHandlerPtr& handler) const
{
#ifdef ICEE_HAS_ROUTER
    if(_routerInfo)
    {
        //
        // If we route, we send everything to the router's client
        // proxy endpoints.
        //
        _routerInfo->getClientEndpoints(handler);
        return;
    }
#endif
    
    getConnectionNoRouterInfo(handler);
}

void
IceInternal::RoutableReference::getConnectionNoRouterInfo(const ConnectRequestHandlerPtr& handler) const
{
    if(!_endpoints.empty())
    {
        createConnection(_endpoints, handler);
        return;
    }

#ifdef ICEE_HAS_LOCATOR
    if(_locatorInfo)
    {
        _locatorInfo->getEndpoints(const_cast<RoutableReference*>(this), handler);
        return;
    }
#endif
    
    handler->setException(Ice::NoEndpointException(__FILE__, __LINE__, toString()));
}

void
IceInternal::RoutableReference::createConnection(const vector<EndpointPtr>& allEndpoints, 
                                                 const ConnectRequestHandlerPtr& handler) const
{
    vector<EndpointPtr> endpoints = filterEndpoints(allEndpoints);
    if(endpoints.empty())
    {
        handler->setException(Ice::NoEndpointException(__FILE__, __LINE__, toString()));
        return;
    }

    getInstance()->outgoingConnectionFactory()->create(endpoints, handler);
}

#endif // ICEE_HAS_AMI

void
IceInternal::RoutableReference::applyOverrides(vector<EndpointPtr>& endpts) const
{
    //
    // Apply the endpoint overrides to each endpoint.
    //
    for(vector<EndpointPtr>::iterator p = endpts.begin(); p != endpts.end(); ++p)
    {
        if(_overrideTimeout)
        {
            *p = (*p)->timeout(_timeout);
        }
    }
}

IceInternal::RoutableReference::RoutableReference(const RoutableReference& r) :
    Reference(r),
    _endpoints(r._endpoints),
    _adapterId(r._adapterId),
#ifdef ICEE_HAS_LOCATOR
    _locatorInfo(r._locatorInfo),
#endif
#ifdef ICEE_HAS_ROUTER
    _routerInfo(r._routerInfo),
#endif
    _overrideTimeout(r._overrideTimeout),
    _timeout(r._timeout)
{
}

vector<EndpointPtr>
IceInternal::RoutableReference::filterEndpoints(const vector<EndpointPtr>& allEndpoints) const
{
    vector<EndpointPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::unknown)),
                    endpoints.end());

    //
    // Filter out endpoints according to the mode of the reference.
    //
    switch(getMode())
    {
        case ReferenceModeTwoway:
        case ReferenceModeOneway:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchOneway:
#endif
        {
            //
            // Filter out datagram endpoints.
            //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), Ice::constMemFun(&Endpoint::datagram)),
                            endpoints.end());
            break;
        }

        case ReferenceModeDatagram:
#ifdef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
#endif
        {
            //
            // Filter out non-datagram endpoints.
            //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(Ice::constMemFun(&Endpoint::datagram))),
                            endpoints.end());
            break;
        }

#ifndef ICEE_HAS_BATCH
        case ReferenceModeBatchDatagram:
        case ReferenceModeBatchOneway:
        {
            throw FeatureNotSupportedException(__FILE__, __LINE__, "batch proxy mode");
        }
#endif
    }

    //
    // Randomize the order of endpoints.
    //
    RandomNumberGenerator rng;
    random_shuffle(endpoints.begin(), endpoints.end(), rng);

    //
    // If a secure connection is requested or secure overrides is set,
    // remove all non-secure endpoints. Otherwise make non-secure
    // endpoints preferred over secure endpoints by partitioning
    // the endpoint vector, so that non-secure endpoints come
    // first.
    //
    // NOTE: we don't use the stable_partition algorithm from STL to
    // keep the code size down.
    //
    vector<EndpointPtr>::iterator p = endpoints.begin();
    vector<EndpointPtr> secureEndpoints;
    while(p != endpoints.end())
    {
        if((*p)->secure())
        {
            secureEndpoints.push_back(*p);
            p = endpoints.erase(p);
        }
        else
        {
            ++p;
        }
    }
    if(getSecure())
    {
        endpoints.swap(secureEndpoints);
    }
    else
    {
        endpoints.insert(endpoints.end(), secureEndpoints.begin(), secureEndpoints.end());
    }

    return endpoints;
}
