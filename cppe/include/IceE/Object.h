// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_H
#define ICEE_OBJECT_H

#include <IceE/ObjectF.h>
#include <IceE/Current.h>

namespace IceInternal
{

class Incoming;
class BasicStream;

}

namespace Ice
{

enum DispatchStatus
{
    DispatchOK,
    DispatchUserException
};

class ICE_API Request
{
public:

    virtual ~Request() {}
    virtual const Current& getCurrent() = 0;
};


class ICE_API Object : virtual public IceUtil::Shared
{
public:

    virtual bool operator==(const Object&) const;
    virtual bool operator<(const Object&) const;

    virtual Int ice_hash() const;

    virtual bool ice_isA(const std::string&, const Current& = Current()) const;
    virtual void ice_ping(const Current&  = Current()) const;
    virtual std::vector< std::string> ice_ids(const Current& = Current()) const;
    virtual const std::string& ice_id(const Current& = Current()) const;

    static const std::string& ice_staticId();

    virtual ObjectPtr ice_clone() const;

#ifndef ICEE_PURE_CLIENT
    DispatchStatus ___ice_isA(IceInternal::Incoming&, const Current&);
    DispatchStatus ___ice_ping(IceInternal::Incoming&, const Current&);
    DispatchStatus ___ice_ids(IceInternal::Incoming&, const Current&);
    DispatchStatus ___ice_id(IceInternal::Incoming&, const Current&);

    static std::string __all[];
    virtual DispatchStatus ice_dispatch(Request&);
    virtual DispatchStatus __dispatch(IceInternal::Incoming&, const Current&);
#endif

    virtual void ice_preMarshal();
    virtual void ice_postUnmarshal();

    virtual void __write(IceInternal::BasicStream*) const;
    virtual void __read(IceInternal::BasicStream*, bool);

protected:

    Object() {};
    virtual ~Object() {} // This class is abstract.

#ifndef ICEE_PURE_CLIENT
    static void __checkMode(OperationMode expected, OperationMode received) // Inline for performance reasons.
    {
        if(expected != received)
        {
            __invalidMode(expected, received); // Not inlined.
        }
    }
    static void __invalidMode(OperationMode, OperationMode);
#endif
};

}

#endif
