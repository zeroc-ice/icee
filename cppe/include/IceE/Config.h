// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONFIG_H
#define ICEE_CONFIG_H

#include <IceE/Features.h>

//
// Endianness
//
// Most CPUs support only one endianness, with the notable exceptions
// of Itanium (IA64) and MIPS.
//
#if defined(__i386)  || defined(_M_IX86)    || defined (__x86_64) || \
    defined(_M_X64)  || defined(_M_IA64)    || defined(__alpha__) || \
    defined (_M_ARM) || defined(__MIPSEL__) || defined (__ARMEL__) || \
    defined (__BFIN__)
#   define ICE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa) || \
      defined(__ppc__) || defined(__powerpc) || defined(_ARCH_COM) || \
      defined(__MIPSEB__)
#   define ICE_BIG_ENDIAN
#else
#    error "Unknown architecture"
#endif

//
// 32 or 64 bit mode?
//
#if defined(__linux) && defined(__sparc__)
//
// We are a linux sparc, which forces 32 bit usr land, no matter the architecture
//
#   define  ICE_32
#elif defined(__sun) && (defined(__sparcv9) || defined(__x86_64))  || \
      defined(__linux) && defined(__x86_64)                        || \
      defined(__hppa) && defined(__LP64__)                         || \
      defined(_ARCH_COM) && defined(__64BIT__)                     || \
      defined(__alpha__)                                           || \
      defined(_WIN64)
#   define ICE_64
#else
#   define ICE_32
#endif

//
// Compiler extensions to export and import symbols: see the documentation 
// for Visual C++, Sun ONE Studio 8 and HP aC++.
//
// TODO: more macros to support IBM Visual Age _Export syntax as well.
//
#if ((defined(_MSC_VER) || defined(_WIN32_WCE)) && !defined(ICEE_STATIC_LIBS)) || (defined(__HP_aCC) && defined(__HP_WINDLL))
#   define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICE_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#   define ICE_DECLSPEC_EXPORT __global
#   define ICE_DECLSPEC_IMPORT
#else
#   define ICE_DECLSPEC_EXPORT /**/
#   define ICE_DECLSPEC_IMPORT /**/
#endif

#if defined(_MSC_VER)
#   define ICE_DEPRECATED_API __declspec(deprecated)
#elif defined(__GNUC__)
#   define ICE_DEPRECATED_API __attribute__((deprecated))
#else
#   define ICE_DEPRECATED_API /**/
#endif

//
// Let's use these extensions with IceE:
//
#ifdef ICE_API_EXPORTS
#   define ICE_API ICE_DECLSPEC_EXPORT
#else
#   define ICE_API ICE_DECLSPEC_IMPORT
#endif

#ifdef _WIN32

#   ifndef _WIN32_WCE
#      ifndef _WIN32_WINNT
           //
           // Necessary for TryEnterCriticalSection (see IceE/Mutex.h).
           //
#          if defined(_MSC_VER) && _MSC_VER < 1500
#             define _WIN32_WINNT 0x0400
#          endif
#      elif _WIN32_WINNT < 0x0400
#         error "TryEnterCricalSection requires _WIN32_WINNT >= 0x0400"
#      endif

#   endif


#   include <windows.h>

#   if defined(_WIN32_WCE) && defined(_MSC_VER)
       //
       // return type for ... (ie; not a UDT or reference to a UDT.  Will
       // produce errors if applied using infix notation)
       //
#      pragma warning( disable : 4284 )
#   endif

// '...' : forcing value to bool 'true' or 'false' (performance warning)
#   pragma warning( disable : 4800 )
// ... identifier was truncated to '255' characters in the debug information
#   pragma warning( disable : 4786 )
// 'this' : used in base member initializer list
#   pragma warning( disable : 4355 )
// class ... needs to have dll-interface to be used by clients of class ...
#   pragma warning( disable : 4251 )
// ... : inherits ... via dominance
#   pragma warning( disable : 4250 )
// non dll-interface class ... used as base for dll-interface class ...
#   pragma warning( disable : 4275 )
//  ...: decorated name length exceeded, name was truncated
#   pragma warning( disable : 4503 )  
#endif

//
// Some include files we need almost everywhere.
//
#include <cassert>

#include <string>

#ifndef _WIN32
#   include <pthread.h>
#   include <errno.h>
#endif

//
// By deriving from this class, other classes are made non-copyable.
//
namespace IceUtil
{

//
// TODO: Constructor and destructor should not be inlined, as they are
// not performance critical.
//

class noncopyable
{
protected:

    noncopyable() { }
    ~noncopyable() { } // May not be virtual! Classes without virtual operations also derive from noncopyable.

private:

    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

//
// Int64 typedef
//
#if defined(__BCPLUSPLUS__) || defined(_MSC_VER)
//
// On Windows, long is always 32-bit
//
typedef __int64 Int64;
#elif defined(ICE_64)
typedef long Int64;
#else
typedef long long Int64;
#endif

}

//
// ICE_INT64: macro for Int64 literal values
//
#if defined(__BCPLUSPLUS__) || defined(_MSC_VER)
#   define ICE_INT64(n) n##i64
#elif defined(ICE_64)
#   define ICE_INT64(n) n##L
#else
#   define ICE_INT64(n) n##LL
#endif

//
// The Ice-E version.
//
#define ICEE_STRING_VERSION "1.3.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICEE_INT_VERSION 10300      // AABBCC, with AA=major, BB=minor, CC=patch

//
// Some include files we need almost everywhere
//
#if !defined(_WIN32_WCE)
#include <cerrno>
#endif
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>

//
// Define the IceInternal namespace, so that we can use the following
// everywhere in our code:
//
// using namespace IceInternal;
//

namespace IceInternal
{
}

namespace Ice
{

typedef unsigned char Byte;
typedef short Short;
typedef int Int;
typedef IceUtil::Int64 Long;
typedef float Float;
typedef double Double;

#ifdef ICEE_HAS_WSTRING
typedef std::wstring Wstring;
#else
typedef std::string Wstring;
#endif

}

// TODO: Should not be inline, this is not performance critical.
#ifdef _WIN32
inline int getSystemErrno() { return GetLastError(); }
#else
inline int getSystemErrno() { return errno; }
#endif

// Epoll not available on gumstix
#ifdef GUMSTIX
#define ICE_NO_EPOLL 1
#endif

#endif
