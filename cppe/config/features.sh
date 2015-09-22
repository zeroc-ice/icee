#!/bin/bash

#
# This script emits Features.h. The path name of Features.h must be specified as
# the first argument. Additional arguments indicate which features have been
# enabled, for example:
#
# features.sh include/IceE/Features.h HAS_ROUTER=yes HAS_LOCATOR=yes ...
#

if [ $# -lt 1 ]; then
    echo "No output filename specified."
    exit 1
fi

targ=$1
shift

HAS_ROUTER=//
HAS_LOCATOR=//
HAS_BATCH=//
HAS_WSTRING=//
HAS_OPAQUE_ENDPOINTS=//
HAS_AMI=//
DEFAULT_MUTEX_PROTOCOL=PrioNone

for arg; do
    if [ "HAS_ROUTER=yes" == "$arg" ]; then HAS_ROUTER=; fi
    if [ "HAS_LOCATOR=yes" == "$arg" ]; then HAS_LOCATOR=; fi
    if [ "HAS_BATCH=yes" == "$arg" ]; then HAS_BATCH=; fi
    if [ "HAS_WSTRING=yes" == "$arg" ]; then HAS_WSTRING=; fi
    if [ "HAS_OPAQUE_ENDPOINTS=yes" == "$arg" ]; then HAS_OPAQUE_ENDPOINTS=; fi
    if [ "HAS_AMI=yes" == "$arg" ]; then HAS_AMI=; fi
    if [ "DEFAULT_MUTEX_PROTOCOL=PrioInherit" == "$arg" ]; then DEFAULT_MUTEX_PROTOCOL=PrioInherit; fi
done

cat > $targ <<HERE
// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// DO NOT EDIT - This file is generated automatically!

#ifndef ICEE_FEATURES_H
#define ICEE_FEATURES_H

${HAS_ROUTER}#define ICEE_HAS_ROUTER
${HAS_LOCATOR}#define ICEE_HAS_LOCATOR
${HAS_BATCH}#define ICEE_HAS_BATCH
${HAS_WSTRING}#define ICEE_HAS_WSTRING
${HAS_OPAQUE_ENDPOINTS}#define ICEE_HAS_OPAQUE_ENDPOINTS
${HAS_AMI}#define ICEE_HAS_AMI

#ifndef _WIN32
    #define ICEE_DEFAULT_MUTEX_PROTOCOL ${DEFAULT_MUTEX_PROTOCOL} 
#endif

#endif
HERE
