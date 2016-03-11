# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_MODULE            = TestDerived
LOCAL_PATH              = cpp/test/Ice/objects

LOCAL_SLICES            = Derived.ice \
                          DerivedEx.ice

include $(TEST_STATICLIBRARY_RULES)

include $(CLEAR_RULES)
cpp_test_Ice_objects_client:: cpp_test_Ice_objects_TestDerived_staticlib

LOCAL_PATH                      = cpp/test/Ice/objects
TEST_LDFLAGS					= -L$(LOCAL_PATH)/$(LOCAL_OBJPREFIX)
TEST_LINKWITH					= -lTestDerived
CLIENT_SRCS_EXT                 = TestI.cpp
SERVER_SRCS_EXT					= TestIntfI.cpp
COLLOC_SRCS_EXT					= $(SERVER_SRCS_EXT)
TEST_TARGET_EXT                 = colloc
include $(CLIENTSERVERTEST_RULES)
