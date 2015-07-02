# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

LOCAL_PATH	= cpp/test/Ice/adapterDeactivation

TEST_TARGET_EXT	= colloc

SERVER_SRCS_EXT	= ServantLocatorI.cpp

include $(CLIENTSERVERTEST_RULES)
