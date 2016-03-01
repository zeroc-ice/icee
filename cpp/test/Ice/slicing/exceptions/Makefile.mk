# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)
LOCAL_PATH                      = cpp/test/Ice/slicing/exceptions
TEST_TARGET_EXT                 = serveramd
CLIENT_SLICES_EXT               = ClientPrivate.ice
SERVER_SLICES_EXT               = ServerPrivate.ice
AMD_SLICES_EXT                  = ServerPrivateAMD.ice
include $(CLIENTSERVERTEST_RULES)
