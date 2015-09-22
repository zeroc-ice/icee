#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.abspath(toplevel)
    if os.path.exists(os.path.join(toplevel, "scripts", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel))
from scripts import *

#
# List of all basic tests.
#
tests = [ 
    ("IceE/thread", ["once"]),
    ("IceE/uuid", ["once"]),
    ("IceE/proxy", ["core"]),
    ("IceE/operations", ["core"]),
    ("IceE/exceptions", ["core"]),
    ("IceE/inheritance", ["core"]),
    ("IceE/facets", ["core"]),
    ("IceE/binding", ["core"]),
    ("IceE/timeout", ["core"]),
    ("IceE/faultTolerance", ["core"]),
    ("IceE/location", ["core"]),
    ("IceE/adapterDeactivation", ["core"]),
    ("IceE/objects", ["core"]),
    ("IceE/slicing/exceptions", ["core"]),
    ("IceE/slicing/objects", ["core"]),
    ("IceE/custom", ["core"]),
    ("IceE/retry", ["core"]),
    ("IceE/defaultServant", ["core"]),
    ("IceE/interceptor", ["core"])
    ]

#
# Run priority tests only if running as root on Unix.
#
if TestUtil.isWin32() or os.getuid() == 0:
    tests += [
        ("IceE/priority", ["core"]),
        ("IceE/threadPoolPriority", ["core"])
        ]

if __name__ == "__main__":
    TestUtil.run(tests)
