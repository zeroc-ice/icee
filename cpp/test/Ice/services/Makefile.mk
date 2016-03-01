# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_EXE                       = client
LOCAL_PATH                      = cpp/test/Ice/services
LOCAL_SLICES                    = Test.ice
LOCAL_SRCS                      = Client.cpp \
                                  AllTests.cpp
LOCAL_DEPENDENT_MODULES         = IceStorm IceGrid Glacier2
include $(TEST_APPLICATION_RULES)
