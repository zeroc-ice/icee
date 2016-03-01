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
    LOCAL_EXE                   = client
    LOCAL_PATH                  = cpp/test/IceBox/admin
    LOCAL_SLICES                = Test.ice
    LOCAL_SRCS                  = Client.cpp AllTests.cpp
    include $(TEST_APPLICATION_RULES)

    include $(CLEAR_RULES)
    LOCAL_MODULE                = TestService
    LOCAL_PATH                  = cpp/test/IceBox/admin
    LOCAL_SLICES                = Test.ice
    LOCAL_SRCS                  = TestI.cpp Service.cpp
    LOCAL_DEPENDENT_DYMODULES   = IceBox Ice IceUtil
    LOCAL_RESOURCES             = config.icebox config.service
    include $(TEST_DYNAMICLIBRARY_RULES)
endif