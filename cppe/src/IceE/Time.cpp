// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/Time.h>
#include <IceE/LocalException.h>

#ifndef _WIN32_WCE
#   if defined(_WIN32)
#       include <sys/timeb.h>
#       include <time.h>
#   else
#       include <sys/time.h>
#   endif
#endif

using namespace IceUtil;

#ifdef _WIN32

namespace
{

static double frequency = -1.0;

//
// Initialize the frequency
//
class InitializeFrequency
{
public:

    InitializeFrequency()
    {
        //
        // Get the frequency of performance counters. We also make a call to
        // QueryPerformanceCounter to ensure it works. If it fails or if the
        // call to QueryPerformanceFrequency fails, the frequency will remain
        // set to -1.0 and ftime will be used instead.
        //
        Int64 v;
        if(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&v)))
        {
            if(QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&v)))
            {
                frequency = static_cast<double>(v);
            }
        }
    }
};
static InitializeFrequency frequencyInitializer;

};
#endif


IceUtil::Time::Time() :
    _usec(0)
{
}

Time
IceUtil::Time::now(Clock clock)
{
#if defined(_WIN32_WCE)
    //
    // Note that GetTickCount returns the number of ms since the
    // device was started. Time cannot be used to represent an
    // absolute time on CE since GetLocalTime doesn't have millisecond
    // resolution.
    //
    return Time(static_cast<Int64>(GetTickCount()) * 1000);
#else
    if(clock == Realtime)
    {
#  if defined(_WIN32)
        struct _timeb tb;
        _ftime(&tb);
        return Time(static_cast<Int64>(tb.time) * ICE_INT64(1000000) + tb.millitm * 1000);
#  else
        struct timeval tv;
        if(gettimeofday(&tv, 0) < 0)
        {
            assert(0);
            throw Ice::SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(tv.tv_sec * ICE_INT64(1000000) + tv.tv_usec);
#  endif
    }
    else
    {
#if defined(_WIN32)
        if(frequency > 0.0)
        {
            Int64 count;
            if(!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&count)))
            {
                assert(0);
                throw Ice::SyscallException(__FILE__, __LINE__, GetLastError());
            }
            return Time(static_cast<Int64>(count / frequency * 1000000.0));
        }
        else
        {
            struct _timeb tb;
            _ftime(&tb);
            return Time(static_cast<Int64>(tb.time) * ICE_INT64(1000000) + tb.millitm * 1000);
        }
#elif defined(__hpux) || defined(__APPLE__)
        //
        // Platforms do not support CLOCK_MONOTONIC
        //
        struct timeval tv;
        if(gettimeofday(&tv, 0) < 0)
        {
            assert(0);
            throw Ice::SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(tv.tv_sec * ICE_INT64(1000000) + tv.tv_usec);
#else
        struct timespec ts;
        if(clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        {
            assert(0);
            throw Ice::SyscallException(__FILE__, __LINE__, errno);
        }
        return Time(ts.tv_sec * ICE_INT64(1000000) + ts.tv_nsec / ICE_INT64(1000));
#endif

    }
#endif
}

Time
IceUtil::Time::seconds(Int64 t)
{
    return Time(t * static_cast<Int64>(1000000));
}

Time
IceUtil::Time::milliSeconds(Int64 t)
{
    return Time(t * static_cast<Int64>(1000));
}

Time
IceUtil::Time::microSeconds(Int64 t)
{
    return Time(t);
}

#ifndef _WIN32
IceUtil::Time::operator timeval() const
{
    timeval tv;
    tv.tv_sec = static_cast<long>(_usec / 1000000);
    tv.tv_usec = static_cast<long>(_usec % 1000000);
    return tv;
}
#endif

Int64
IceUtil::Time::toSeconds() const
{
    return _usec / 1000000;
}

Int64
IceUtil::Time::toMilliSeconds() const
{
    return _usec / 1000;
}

Int64
IceUtil::Time::toMicroSeconds() const
{
    return _usec;
}

double
IceUtil::Time::toSecondsDouble() const
{
    return _usec / 1000000.0;
}

double
IceUtil::Time::toMilliSecondsDouble() const
{
    return _usec / 1000.0;
}

double
IceUtil::Time::toMicroSecondsDouble() const
{
    return static_cast<double>(_usec);
}

Time::Time(Int64 usec) :
    _usec(usec)
{
}

