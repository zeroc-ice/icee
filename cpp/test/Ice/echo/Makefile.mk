# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_EXE                       = server
LOCAL_PATH                      = cpp/test/Ice/echo

LOCAL_SLICES                    = Test.ice

LOCAL_SRCS                      = BlobjectI.cpp \
                                  Server.cpp
include $(TEST_APPLICATION_RULES)
