# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_PATH                      = cpp/test/Ice/servantLocator
TEST_TARGET_EXT                 = colloc serveramd
TEST_SRCS_EXT                   = ServantLocatorI.cpp
CLIENT_SRCS_EXCLUDE             = $(TEST_SRCS_EXT)
include $(CLIENTSERVERTEST_RULES)
