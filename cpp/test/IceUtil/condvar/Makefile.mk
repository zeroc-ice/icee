# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_EXE                       = workqueue
LOCAL_PATH                      = cpp/test/IceUtil/condvar
LOCAL_SRCS                      = WorkQueue.cpp
include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                       = match
LOCAL_PATH                      = cpp/test/IceUtil/condvar
LOCAL_SRCS                      = Match.cpp
include $(TEST_APPLICATION_RULES)
