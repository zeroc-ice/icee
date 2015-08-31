# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_EXE                       = client
LOCAL_PATH                      = cpp/test/Ice/defaultServant

LOCAL_SLICES                    = Test.ice

LOCAL_SRCS                      = Client.cpp \
                                  TestI.cpp \
                                  AllTests.cpp

include $(TEST_APPLICATION_RULES)
