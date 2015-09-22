// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LoggerI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::LoggerI::LoggerI(const string& prefix)
{
    if(!prefix.empty())
    {
        _prefix = prefix + ": ";
    }
#ifdef _WIN32_WCE
    _file = fopen("\\IceE_log.txt", "a");
    if(_file == NULL)
    {
        _file = stderr;
        error("Could not open log file: IceE.log\n");
    }
#else
    _file = stderr;
#endif
}

#ifdef _WIN32_WCE
Ice::LoggerI::~LoggerI()
{
    fclose(_file);
}
#endif

void
Ice::LoggerI::print(const string& message)
{
    fprintf(_file, "%s\n", message.c_str());
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    string s = "[ ";
    s += _prefix;
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += message + " ]";

    string::size_type idx = 0;
    while((idx = s.find("\n", idx)) != string::npos)
    {
        s.insert(idx + 1, "  ");
        ++idx;
    }

    fprintf(_file, "%s\n", s.c_str());
}

void
Ice::LoggerI::warning(const string& message)
{
    fprintf(_file, "%swarning: %s\n", _prefix.c_str(), message.c_str());
}

void
Ice::LoggerI::error(const string& message)
{
    fprintf(_file, "%serror: %s\n", _prefix.c_str(), message.c_str());
}
