# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH	= cpp/test/Ice/retry

TEST_TARGET_EXT	= colloc
CLIENT_SRCS_EXT	= InstrumentationI.cpp

include $(CLIENTSERVERTEST_RULES)
