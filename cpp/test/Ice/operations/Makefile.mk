# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_PATH                      = cpp/test/Ice/operations
TEST_TARGET_EXT                 = colloc serveramd
CLIENT_SRCS_EXT                 = Twoways.cpp \
                                  Oneways.cpp \
                                  TwowaysAMI.cpp \
                                  OnewaysAMI.cpp \
                                  BatchOneways.cpp \
                                  BatchOnewaysAMI.cpp
include $(CLIENTSERVERTEST_RULES)
