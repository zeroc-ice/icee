// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_FACTORY_TABLE_DEF_H
#define ICEE_FACTORY_TABLE_DEF_H

#include <IceE/UserExceptionFactoryF.h>
#include <IceE/ObjectFactoryF.h>
#include <IceE/Mutex.h>

namespace IceInternal
{

class ICE_API FactoryTableDef : private IceUtil::noncopyable
{
public:

    void addExceptionFactory(const ::std::string&, const IceInternal::UserExceptionFactoryPtr&);
    IceInternal::UserExceptionFactoryPtr getExceptionFactory(const ::std::string&) const;
    void removeExceptionFactory(const ::std::string&);

    void addObjectFactory(const ::std::string&, const Ice::ObjectFactoryPtr&);
    Ice::ObjectFactoryPtr getObjectFactory(const ::std::string&) const;
    void removeObjectFactory(const ::std::string&);

private:

    IceUtil::Mutex _m;

    //
    // Code size optimization: instead of defining 2 different maps, we only 
    // define one.
    //
//     typedef ::std::pair<IceInternal::UserExceptionFactoryPtr, int> EFPair;
//     typedef ::std::map< ::std::string, EFPair> EFTable;
//     EFTable _eft;

//     typedef ::std::pair<Ice::ObjectFactoryPtr, int> OFPair;
//     typedef ::std::map< ::std::string, OFPair> OFTable;
//     OFTable _oft;

    typedef ::std::pair<IceUtil::Handle<IceUtil::Shared>, int> FPair;
    typedef ::std::map< ::std::string, FPair> FTable;
    FTable _eft;
    FTable _oft;
};

}

#endif
