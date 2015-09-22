// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/FactoryTable.h>
#include <IceE/UserExceptionFactory.h>
#include <IceE/MutexPtrLock.h>

namespace IceInternal
{

//
// Single global instance of the factory table for non-local
// exceptions and non-abstract classes.
//
ICE_DECLSPEC_EXPORT FactoryTableDef* factoryTable;

}

namespace
{

static int initCount = 0;  // Initialization count
IceUtil::Mutex* initCountMutex = 0;

class Init
{
public:

    Init()
    {
        initCountMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete initCountMutex;
        initCountMutex = 0;
    }
};
Init init;

}

//
// This constructor initializes the single global
// IceInternal::factoryTable instance from the outside (if it hasn't
// been initialized yet). The constructor here is triggered by a
// file-static instance of FactoryTable in each slice2cpp-generated
// header file that uses non-local exceptions or non-abstract classes.
// This ensures that IceInternal::factoryTable is always initialized
// before it is used.
//
IceInternal::FactoryTable::FactoryTable()
{
    IceUtilInternal::MutexPtrLock lock(initCountMutex);
    if(0 == initCount++)
    {
        factoryTable = new FactoryTableDef;
    }
}

//
// The destructor decrements the reference count and, once the
// count drops to zero, deletes the table.
//
IceInternal::FactoryTable::~FactoryTable()
{
    IceUtilInternal::MutexPtrLock lock(initCountMutex);
    if(0 == --initCount)
    {
        delete factoryTable;
    }
}
