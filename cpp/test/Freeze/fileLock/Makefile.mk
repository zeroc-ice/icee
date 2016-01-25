# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_EXE                   = client
LOCAL_PATH                  = cpp/test/Freeze/fileLock
LOCAL_SRCS                  = Client.cpp
LOCAL_DEPENDENT_MODULES     = Freeze
LOCAL_RESOURCES             = db
include $(TEST_APPLICATION_RULES)

include $(CLEAR_RULES)
LOCAL_EXE                   = clientFail
LOCAL_PATH                  = cpp/test/Freeze/fileLock
LOCAL_SRCS                  = ClientFail.cpp
LOCAL_DEPENDENT_MODULES     = Freeze
include $(TEST_APPLICATION_RULES)
