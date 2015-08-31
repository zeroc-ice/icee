# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_PATH                      = cpp/test/Ice/threadPoolPriority
include $(CLIENTSERVERTEST_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                       = servercustom
LOCAL_PATH                      = cpp/test/Ice/threadPoolPriority
LOCAL_SLICES                    = Test.ice
LOCAL_SRCS                      = ServerCustomThreadPool.cpp \
                                  TestI.cpp
include $(TEST_APPLICATION_RULES)
