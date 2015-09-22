::
:: This script emits Features.h. The path name of Features.h must be specified as
:: the first argument. Additional arguments indicate which features have been
:: enabled, for example:
::
:: features.bat include\IceE\Features.h "HAS_ROUTER=yes" "HAS_LOCATOR=yes" ...
::
:: Note that the features must be enclosed in quotes.
::

@echo off
if (%1) == () goto FAIL

set targ=%1
shift

set HAS_ROUTER=//
set HAS_LOCATOR=//
set HAS_BATCH=//
set HAS_WSTRING=//
set HAS_OPAQUE_ENDPOINTS=//
set HAS_AMI=//

for %%n in (%1 %2 %3 %4 %5 %6 %7 %8 %9) do (
if %%n=="HAS_ROUTER=yes" set HAS_ROUTER=
if %%n=="HAS_LOCATOR=yes" set HAS_LOCATOR=
if %%n=="HAS_BATCH=yes" set HAS_BATCH=
if %%n=="HAS_WSTRING=yes" set HAS_WSTRING=
if %%n=="HAS_OPAQUE_ENDPOINTS=yes" set HAS_OPAQUE_ENDPOINTS=
if %%n=="HAS_AMI=yes" set HAS_AMI=
)

echo // **********************************************************************> %targ%
echo //>> %targ%
echo // Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.>> %targ%
echo //>> %targ%
echo // This copy of Ice-E is licensed to you under the terms described in the>> %targ%
echo // ICEE_LICENSE file included in this distribution.>> %targ%
echo //>> %targ%
echo // **********************************************************************>> %targ%
echo.>> %targ%
echo // DO NOT EDIT - This file is generated automatically!>> %targ%
echo.>> %targ%
echo #ifndef ICEE_FEATURES_H>> %targ%
echo #define ICEE_FEATURES_H>> %targ%
echo.>> %targ%
echo %HAS_ROUTER%#define ICEE_HAS_ROUTER>> %targ%
echo %HAS_LOCATOR%#define ICEE_HAS_LOCATOR>> %targ%
echo %HAS_BATCH%#define ICEE_HAS_BATCH>> %targ%
echo %HAS_WSTRING%#define ICEE_HAS_WSTRING>> %targ%
echo %HAS_OPAQUE_ENDPOINTS%#define ICEE_HAS_OPAQUE_ENDPOINTS>> %targ%
echo %HAS_AMI%#define ICEE_HAS_AMI>> %targ%
echo.>> %targ%
echo #ifndef _WIN32>> %targ%
echo #define ICEE_DEFAULT_MUTEX_PROTOCOL PrioNone>> %targ%
echo #endif>> %targ%
echo.>> %targ%
echo #endif>> %targ%

goto DONE

:FAIL
echo No output filename specified.

:DONE
