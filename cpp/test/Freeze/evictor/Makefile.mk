# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(BUILD_TESTSUITE),dynamic)
    include $(CLEAR_RULES)
    LOCAL_EXE                   = client
    LOCAL_PATH                  = cpp/test/Freeze/evictor
    LOCAL_SLICES                = Test.ice
    LOCAL_SRCS                  = Client.cpp
    LOCAL_RESOURCES             = db config
    include $(TEST_APPLICATION_RULES)

    include $(CLEAR_RULES)
    LOCAL_EXE                   = server
    LOCAL_PATH                  = cpp/test/Freeze/evictor
    LOCAL_SLICES                = Test.ice
    LOCAL_SRCS                  = Server.cpp TestI.cpp
    LOCAL_DEPENDENT_MODULES     = Freeze
    include $(TEST_APPLICATION_RULES)
endif
