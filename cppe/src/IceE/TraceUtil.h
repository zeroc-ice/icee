// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRACE_UTIL_H
#define ICEE_TRACE_UTIL_H

#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>

namespace IceInternal
{

class BasicStream;

void traceSend(const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceRecv(const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void trace(const char*, const BasicStream&, const ::Ice::LoggerPtr&, const TraceLevelsPtr&);
void traceSlicing(const char*, const ::std::string&, const char *, const ::Ice::LoggerPtr&);

}

#endif
