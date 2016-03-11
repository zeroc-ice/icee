# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(BUILD_TESTSUITE),dynamic)

    include $(CLEAR_RULES)
    LOCAL_EXE                   = publisher
    LOCAL_PATH                  = cpp/test/IceStorm/federation
    LOCAL_SLICES                = Event.ice
    LOCAL_SRCS                  = Publisher.cpp
    LOCAL_DEPENDENT_MODULES     = IceStorm Ice IceUtil
    LOCAL_RESOURCES             = db 0.db 1.db 2.db
    include $(TEST_APPLICATION_RULES)

    include $(CLEAR_RULES)
    LOCAL_EXE                   = subscriber
    LOCAL_PATH                  = cpp/test/IceStorm/federation
    LOCAL_SLICES                = Event.ice
    LOCAL_SRCS                  = Subscriber.cpp
    LOCAL_DEPENDENT_MODULES     = IceStorm Ice IceUtil
    include $(TEST_APPLICATION_RULES)

endif
