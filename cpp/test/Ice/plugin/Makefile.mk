# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH	= cpp/test/Ice/plugin
LOCAL_LIB	= TestPlugin
LOCAL_LIBDIR	= $(LOCAL_PATH)
LOCAL_SRCDIR	= ice/$(LOCAL_PATH)
LOCAL_MODULE	= cpp_test_Ice_plugin_TestPlugin
LOCAL_CPPFLAGS	= -I$(LOCAL_PATH) -Iice/cpp/test/include -Iice/$(LOCAL_PATH)
LOCAL_SRCS	= ice/$(LOCAL_PATH)/Plugin.cpp

include $(DYNAMICLIBRARY_RULES)