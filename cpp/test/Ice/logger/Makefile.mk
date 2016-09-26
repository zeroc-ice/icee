# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_EXE                       = client1
LOCAL_PATH                      = cpp/test/Ice/logger
LOCAL_SRCS                      = Client1.cpp
LOCAL_RESOURCES                 = config.client
include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                       = client2
LOCAL_PATH                      = cpp/test/Ice/logger
LOCAL_SRCS                      = Client2.cpp
include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                       = client3
LOCAL_PATH                      = cpp/test/Ice/logger
LOCAL_SRCS                      = Client3.cpp
include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                       = client4
LOCAL_PATH                      = cpp/test/Ice/logger
LOCAL_SRCS                      = Client4.cpp
include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                       = client5
LOCAL_PATH                      = cpp/test/Ice/logger
LOCAL_SRCS                      = Client5.cpp
include $(TEST_APPLICATION_RULES)
