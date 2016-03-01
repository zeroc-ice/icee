# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_EXE                       = client
LOCAL_PATH                      = cpp/test/IceUtil/thread
LOCAL_SRCS                      = TestBase.cpp \
                                  CreateTest.cpp \
                                  AliveTest.cpp \
                                  StartTest.cpp \
                                  SleepTest.cpp \
                                  RecMutexTest.cpp \
                                  MutexTest.cpp \
                                  MonitorMutexTest.cpp \
                                  MonitorRecMutexTest.cpp \
                                  CountDownLatchTest.cpp \
                                  TestSuite.cpp \
                                  Client.cpp
include $(TEST_APPLICATION_RULES)
