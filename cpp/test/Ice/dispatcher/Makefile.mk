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
LOCAL_PATH	= cpp/test/Ice/dispatcher

LOCAL_SLICES	= Test.ice

LOCAL_SRCS      = Client.cpp \
		  Dispatcher.cpp \
		  AllTests.cpp

include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)

LOCAL_EXE	= server
LOCAL_PATH	= cpp/test/Ice/dispatcher

LOCAL_SLICES	= Test.ice

LOCAL_SRCS      = TestI.cpp \
		  Dispatcher.cpp \
		  Server.cpp

include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)

LOCAL_EXE	= colloc
LOCAL_PATH	= cpp/test/Ice/dispatcher

LOCAL_SLICES	= Test.ice

LOCAL_SRCS      = TestI.cpp \
		  Dispatcher.cpp \
		  Collocated.cpp \
		  AllTests.cpp

include $(TEST_APPLICATION_RULES)
