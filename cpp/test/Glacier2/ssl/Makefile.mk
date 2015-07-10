# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_EXE	= client
LOCAL_PATH	= cpp/test/Glacier2/ssl
LOCAL_SRCS	= Client.cpp
LOCAL_LINK_WITH	= Ice Glacier2

include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)

LOCAL_EXE	= server
LOCAL_PATH	= cpp/test/Glacier2/ssl
LOCAL_SRCS	= Server.cpp
LOCAL_LINK_WITH	= Ice Glacier2

include $(TEST_APPLICATION_RULES)