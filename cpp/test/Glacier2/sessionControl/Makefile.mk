# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH                      = cpp/test/Glacier2/sessionControl

CLIENT_SLICES                   = Session.ice

CLIENT_SRCS                     = Client.cpp

SERVER_SLICES                   = Session.ice

SERVER_SRCS                     = SessionI.cpp \
                                  Server.cpp

TEST_DEPENDENT_MODULES          = Glacier2

include $(CLIENTSERVERTEST_RULES)
