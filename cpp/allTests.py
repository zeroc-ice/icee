#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    ("Ice/properties", ["once", "nomingw", "noyocto"]),    # This test requires a UTF-8 locale this isn't
                                                           # supported with yocto core images.
    ("Ice/proxy", ["core", "bt"]),
    ("Ice/operations", ["core", "bt"]),
    ("Ice/exceptions", ["core", "bt"]),
    ("Ice/ami", ["core", "nocompress", "nossl", "bt"]), # This test relies on the socket send() blocking and 
						  # doesn't work well with old OpenSSL versions.
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("Ice/inheritance", ["core", "bt"]),
    ("Ice/facets", ["core", "bt"]),
    ("Ice/objects", ["core", "bt"]),
    ("Ice/optional", ["core", "bt"]),
    ("Ice/binding", ["core", "nosocks", "bt"]),
    ("Ice/faultTolerance", ["core", "novalgrind"]), # valgrind reports leak with aborted servers
    ("Ice/location", ["core"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/slicing/exceptions", ["core" "bt"]),
    ("Ice/slicing/objects", ["core", "bt"]),
    ("Ice/gc", ["once"]),
    ("Ice/dispatcher", ["once"]),
    ("Ice/checksum", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/hold", ["core", "bt"]),
    ("Ice/custom", ["core", "nossl", "nows"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core", "nocompress", "nosocks", "nossl"]), # This test relies on the socket send() blocking and 
						                 # doesn't work well with old OpenSSL versions.
    ("Ice/acm", ["core", "bt"]),
    ("Ice/servantLocator", ["core", "bt"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/stringConverter", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/invoke", ["core"]),
    ("Ice/hash", ["once"]),
    ("Ice/admin", ["core", "noipv6"]),
    ("Ice/metrics", ["core", "nossl", "nows", "noipv6", "nocompress", "nomingw", "nosocks"]),
    ("Ice/enums", ["once", "bt"]),
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

if os.path.isfile(os.path.join(path[0], "cpp", "test", "Ice", "plugin", "plugins", "libTestPlugin.so")):
    tests += [("Ice/plugin", ["core"])]

if os.path.isfile(os.path.join(path[0], "cpp", "test", "IceBox", "admin", "libTestService.so")):
    tests += [("IceBox/admin", ["core", "noipv6", "nomx"])]
    
if os.path.isfile(os.path.join(path[0], "cpp", "test", "IceBox", "configuration", "libTestService.so")):
    tests += [("IceBox/configuration", ["core", "noipv6", "nomx"])]

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
