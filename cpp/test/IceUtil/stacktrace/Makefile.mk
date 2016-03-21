# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(BUILD_TESTSUITE),dynamic)
include $(CLEAR_RULES)
LOCAL_EXE                       = client
LOCAL_PATH                      = cpp/test/IceUtil/stacktrace
LOCAL_SRCS                      = Client.cpp
LOCAL_RESOURCES			= StackTrace.debug.Linux StackTrace.release.Linux
include $(TEST_APPLICATION_RULES)
endif