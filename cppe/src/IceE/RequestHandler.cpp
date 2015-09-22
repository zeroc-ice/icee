// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/RequestHandler.h>
#include <IceE/Reference.h>

using namespace std;
using namespace IceInternal;

ICE_DECLSPEC_EXPORT IceUtil::Shared* IceInternal::upCast(RequestHandler* obj) { return obj; }

RequestHandler::~RequestHandler()
{
}
