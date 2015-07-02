# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH	= cpp/test/Ice/background

TEST_SRCS_EXT	= Configuration.cpp

include $(CLIENTSERVERTEST_RULES)

include $(CLEAR_RULES)

LOCAL_PATH	= cpp/test/Ice/background
LOCAL_LIB	= TestTransport
LOCAL_LIBDIR	= $(LOCAL_PATH)
LOCAL_MODULE	= cpp_test_Ice_background_TestTransport
LOCAL_SLICES	= ice/$(LOCAL_PATH)/Test.ice
LOCAL_CPPFLAGS	= -I$(LOCAL_PATH) -Iice/$(LOCAL_PATH)

LOCAL_SRCS	= Configuration.cpp \
                  Connector.cpp \
                  Acceptor.cpp \
                  EndpointI.cpp \
                  Transceiver.cpp \
                  EndpointFactory.cpp \
                  PluginI.cpp

LOCAL_SRCS	:= $(addprefix ice/$(LOCAL_PATH)/,$(LOCAL_SRCS))

include $(DYNAMICLIBRARY_RULES)