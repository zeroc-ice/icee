// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/UnknownEndpoint.h>
#include <IceE/BasicStream.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/SafeStdio.h>

#ifdef ICEE_HAS_OPAQUE_ENDPOINTS
#  include <IceE/Base64.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifdef ICEE_HAS_OPAQUE_ENDPOINTS

IceInternal::UnknownEndpoint::UnknownEndpoint(const string& str)
{
    const string delim = " \t\n\r";

    string::size_type beg;
    string::size_type end = 0;

    int topt = 0;
    int vopt = 0;

    while(true)
    {
        beg = str.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }

        end = str.find_first_of(delim, beg);
        if(end == string::npos)
        {
            end = str.length();
        }

        string option = str.substr(beg, end - beg);
        if(option.length() != 2 || option[0] != '-')
        if(option.length() != 2 || option[0] != '-')
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "opaque " + str;
            throw ex;
        }

        string argument;
        string::size_type argumentBeg = str.find_first_not_of(delim, end);
        if(argumentBeg != string::npos && str[argumentBeg] != '-')
        {
            beg = argumentBeg;
            end = str.find_first_of(delim, beg);
            if(end == string::npos)
            {
                end = str.length();
            }
            argument = str.substr(beg, end - beg);
        }

        switch(option[1])
        {
            case 't':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "opaque " + str;
                    throw ex;
                }
                for(string::size_type i = 0; i < argument.size(); ++i)
                {
                    if(!isdigit(static_cast<unsigned char>(argument[i])) && (i != 0 || argument[i] != '-'))
                    {
                        EndpointParseException ex(__FILE__, __LINE__);
                        ex.str = "opaque " + str;
                        throw ex;
                    }
                }

                Ice::Int t = atoi(argument.c_str());
                if(t < 0 || t > 65535)
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "opaque " + str;
                    throw ex;
                }
                _type = static_cast<Ice::Short>(t);
                ++topt;
                break;
            }

            case 'v':
            {
                if(argument.empty())
                {
                    EndpointParseException ex(__FILE__, __LINE__);
                    ex.str = "opaque " + str;
                    throw ex;
                }
                for(string::size_type i = 0; i < argument.size(); ++i)
                {
                    if(!Base64::isBase64(argument[i]))
                    {
                        EndpointParseException ex(__FILE__, __LINE__);
                        ex.str = "opaque " + str;
                        throw ex;
                    }
                }
                const_cast<vector<Byte>&>(_rawBytes) = Base64::decode(argument);
                ++vopt;
                break;
            }

            default:
            {
                EndpointParseException ex(__FILE__, __LINE__);
                ex.str = "opaque " + str;
                throw ex;
            }
        }
    }
    if(topt != 1 || vopt != 1)
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "opaque " + str;
        throw ex;
    }
}

#endif

IceInternal::UnknownEndpoint::UnknownEndpoint(Short type, BasicStream* s) :
    _instance(s->instance()),
    _type(type)
{
    s->startReadEncaps();
    Int sz = s->getReadEncapsSize();
    s->readBlob(const_cast<vector<Byte>&>(_rawBytes), sz);
    s->endReadEncaps();
}

void
IceInternal::UnknownEndpoint::streamWrite(BasicStream* s) const
{
    s->write(_type);
    s->startWriteEncaps();
    s->writeBlob(_rawBytes);
    s->endWriteEncaps();
}

string
IceInternal::UnknownEndpoint::toString() const
{
#ifdef ICEE_HAS_OPAQUE_ENDPOINTS
    return Ice::printfToString("opaque -t %d -v %s", _type, Base64::encode(_rawBytes).c_str());
#else
    return string();
#endif
}

Short
IceInternal::UnknownEndpoint::type() const
{
    return _type;
}

Int
IceInternal::UnknownEndpoint::timeout() const
{
    return -1;
}

EndpointPtr
IceInternal::UnknownEndpoint::timeout(Int) const
{
    return const_cast<UnknownEndpoint*>(this);
}

bool
IceInternal::UnknownEndpoint::secure() const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::datagram() const
{
    return false;
}

bool
IceInternal::UnknownEndpoint::unknown() const
{
    return true;
}

#ifndef ICEE_HAS_AMI
vector<ConnectorPtr>
IceInternal::UnknownEndpoint::connectors() const
{
    vector<ConnectorPtr> ret;
    return ret;
}
#else
void
IceInternal::UnknownEndpoint::connectors_async(const Endpoint_connectorsPtr& callback) const
{
    callback->connectors(vector<ConnectorPtr>());
}
#endif

#ifndef ICEE_PURE_CLIENT
AcceptorPtr
IceInternal::UnknownEndpoint::acceptor(EndpointPtr& endp) const
{
    endp = const_cast<UnknownEndpoint*>(this);
    return 0;
}

vector<EndpointPtr>
IceInternal::UnknownEndpoint::expand() const
{
    assert(false);
    vector<EndpointPtr> ret;
    return ret;

}
#endif

bool
IceInternal::UnknownEndpoint::operator==(const Endpoint& r) const
{
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_type != p->_type)
    {
        return false;
    }

    if(_rawBytes != p->_rawBytes)
    {
        return false;
    }

    return true;
}

bool
IceInternal::UnknownEndpoint::operator<(const Endpoint& r) const
{
    const UnknownEndpoint* p = dynamic_cast<const UnknownEndpoint*>(&r);
    if(!p)
    {
        return type() < r.type();
    }

    if(this == p)
    {
        return false;
    }

    if(_type < p->_type)
    {
        return true;
    }
    else if(p->_type < _type)
    {
        return false;
    }

    if(_rawBytes < p->_rawBytes)
    {
        return true;
    }
    else if(p->_rawBytes < _rawBytes)
    {
        return false;
    }

    return false;
}
