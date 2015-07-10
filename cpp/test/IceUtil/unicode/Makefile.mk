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
LOCAL_PATH	= cpp/test/IceUtil/unicode
LOCAL_SRCS	= Client.cpp
LOCAL_RESOURCES	= coeur.utf16be \
		  coeur.utf16le \
		  coeur.utf32be \
		  coeur.utf32le \
		  coeur.utf8 \
		  FDL \
		  filename.txt

include $(TEST_APPLICATION_RULES)
