# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

include $(CLEAR_RULES)

TEST_CPPFLAGS                   = -DSTRING_VIEW_IGNORE_STRING_CONVERTER
LOCAL_PATH                      = cpp/test/Ice/custom
TEST_TARGET_EXT                 = colloc serveramd

TEST_SLICES_EXT                 = Wstring.ice
AMD_SLICES_EXCLUDE              = $(TEST_SLICES_EXT)

TEST_SRCS_EXT                   = MyByteSeq.cpp \
                                  StringConverterI.cpp

SERVER_SRCS_EXT                 = WstringI.cpp

AMD_SLICES_EXT                  = WstringAMD.ice
AMD_SRCS_EXT                    = WstringAMDI.cpp

include $(CLIENTSERVERTEST_RULES)
