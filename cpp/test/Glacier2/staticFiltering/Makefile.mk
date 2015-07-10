# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH	= cpp/test/Glacier2/staticFiltering

CLIENT_SLICES	= Backend.ice

CLIENT_SRCS	= BackendI.cpp \
		  Client.cpp

CLIENT_LINK_WITH = Glacier2

SERVER_SLICES	= Backend.ice

SERVER_SRCS	= BackendI.cpp \
		  Server.cpp

SERVER_LINK_WITH = Glacier2

include $(CLIENTSERVERTEST_RULES)
