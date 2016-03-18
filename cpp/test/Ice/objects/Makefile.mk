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

ifeq ($(BUILD_TESTSUITE),static)
include $(TEST_STATICLIBRARY_RULES)
cpp/test/Ice/objects/server: cpp/test/Ice/objects/libTestDerived$(LIBNAME_SUFFIX).a
cpp/test/Ice/objects/client: cpp/test/Ice/objects/libTestDerived$(LIBNAME_SUFFIX).a
cpp/test/Ice/objects/collocated: cpp/test/Ice/objects/libTestDerived$(LIBNAME_SUFFIX).a
else
include $(TEST_DYNAMICLIBRARY_RULES)
cpp/test/Ice/objects/server: cpp/test/Ice/objects/libTestDerived$(LIBNAME_SUFFIX).so
cpp/test/Ice/objects/client: cpp/test/Ice/objects/libTestDerived$(LIBNAME_SUFFIX).so
cpp/test/Ice/objects/collocated: cpp/test/Ice/objects/libTestDerived$(LIBNAME_SUFFIX).so
endif

include $(CLEAR_RULES)

LOCAL_PATH                      = cpp/test/Ice/objects
TEST_LDFLAGS					= -L$(LOCAL_PATH)
TEST_LINKWITH					= -lTestDerived$(LIBNAME_SUFFIX)
CLIENT_SRCS_EXT                 = TestI.cpp
SERVER_SRCS_EXT					= TestIntfI.cpp
COLLOC_SRCS_EXT					= $(SERVER_SRCS_EXT)
TEST_TARGET_EXT                 = colloc
include $(CLIENTSERVERTEST_RULES)
