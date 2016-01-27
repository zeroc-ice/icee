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
    LOCAL_PATH                  = cpp/test/Ice/plugin
    LOCAL_SRCS                  = Client.cpp
    include $(TEST_APPLICATION_RULES)

    include $(CLEAR_RULES)
    LOCAL_MODULE                = TestPlugin
    LOCAL_PATH                  = cpp/test/Ice/plugin
    LOCAL_LIBDIR                = cpp/test/Ice/plugin/plugins
    LOCAL_SRCS                  = Plugin.cpp
    include $(TEST_DYNAMICLIBRARY_RULES)
endif
