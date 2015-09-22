// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_BASE_64_H
#define ICEE_BASE_64_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_OPAQUE_ENDPOINTS

#include <string>
#include <vector>

namespace IceInternal
{

class Base64
{
public:

    static std::string encode(const std::vector<unsigned char>&);
    static std::vector<unsigned char> decode(const std::string&);
    static bool isBase64(char);

private:

    static char encode(unsigned char);
    static unsigned char decode(char);
};

}

#endif

#endif
