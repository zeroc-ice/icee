#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, getopt

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

#
# List of all basic tests.
#
tests = [
    ("IceUtil/condvar", ["once", "win32only"]),
    ("IceUtil/thread",  ["once"]),
    ("IceUtil/unicode", ["once"]),
    ("IceUtil/inputUtil",  ["once"]),
    ("IceUtil/uuid", ["once", "noappverifier"]), # This test is very slow with appverifier.
    ("IceUtil/timer", ["once"]),
    ("IceUtil/sha1", ["once"]),
    ("Ice/properties", ["once", "nomingw"]),
    ("Ice/proxy", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core", "nocompress", "nossl"]), # This test relies on the socket send() blocking and 
						  # doesn't work well with old OpenSSL versions.
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/optional", ["core"]),
    ("Ice/binding", ["core", "nosocks"]),
    ("Ice/faultTolerance", ["core", "novalgrind"]), # valgrind reports leak with aborted servers
    ("Ice/location", ["core"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/gc", ["once"]),
    ("Ice/dispatcher", ["once"]),
    ("Ice/checksum", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/hold", ["core"]),
    ("Ice/custom", ["core", "nossl", "nows"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core", "nocompress", "nosocks", "nossl"]), # This test relies on the socket send() blocking and 
						                 # doesn't work well with old OpenSSL versions.
    ("Ice/acm", ["core"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/stringConverter", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/invoke", ["core"]),
    ("Ice/hash", ["once"]),
    ("Ice/admin", ["core", "noipv6"]),
    ("Ice/metrics", ["core", "nossl", "nows", "noipv6", "nocompress", "nomingw", "nosocks"]),
    ("Ice/enums", ["once"]),
    ("Ice/logger", ["once"]),
    ("Ice/networkProxy", ["core", "noipv6", "nosocks"]),
    ("Ice/services", ["once"]),
    ("Glacier2/router", ["service", "novc100", "nomingw"]),
    ("Glacier2/attack", ["service", "novc100", "nomingw", "nomx"]),
    ("Glacier2/override", ["service", "novc100", "nomingw"]),
    ("Glacier2/sessionControl", ["service", "novc100", "nomingw"]),
    ("Glacier2/ssl", ["service", "novalgrind", "novc100", "nomingw"]), # valgrind doesn't work well with openssl
    ("Glacier2/dynamicFiltering", ["service", "novc100", "nomingw"]),
    ("Glacier2/staticFiltering", ["service", "noipv6", "novc100", "nomingw", "nomx"]),
    ("Glacier2/sessionHelper", ["core", "novc100", "nomingw"]),
    ("IceDiscovery/simple", ["service"]),
    ]

#
# Run priority tests only if running as root on Unix.
#
if TestUtil.isWin32() or os.getuid() == 0:
    tests += [
        ("IceUtil/priority", ["core", "nodarwin"]),
        ("Ice/threadPoolPriority", ["core", "nodarwin", "nomx"])
        ]

if __name__ == "__main__":
    TestUtil.run(tests)
