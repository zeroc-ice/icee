# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_PATH                      = cpp/test/Ice/objects
TEST_SLICES_EXT                 = Derived.ice DerivedEx.ice
CLIENT_SRCS_EXT                 = TestI.cpp
SERVER_SRCS_EXT                 = TestIntfI.cpp
TEST_TARGET_EXT                 = colloc
include $(CLIENTSERVERTEST_RULES)
