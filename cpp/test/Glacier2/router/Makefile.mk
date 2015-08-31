# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH                      = cpp/test/Glacier2/router

CLIENT_SLICES                   = Callback.ice

CLIENT_SRCS                     = CallbackI.cpp \
                                  Client.cpp

CLIENT_DEPENDENT_MODULES        = Glacier2

SERVER_SLICES                   = Callback.ice

SERVER_SRCS                     = CallbackI.cpp \
                                  Server.cpp

include $(CLIENTSERVERTEST_RULES)
