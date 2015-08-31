# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_PATH                      = cpp/test/Ice/dispatcher
TEST_TARGET_EXT                 = colloc
TEST_SRCS_EXT                   = Dispatcher.cpp
include $(CLIENTSERVERTEST_RULES)