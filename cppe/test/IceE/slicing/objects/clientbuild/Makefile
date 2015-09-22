# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../../..

CLIENT		= ../client

TARGETS		= $(CLIENT)

OBJS		= Test.o \
		  ClientPrivate.o \
		  Forward.o

TOBJS		= Client.o \
		  AllTests.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(patsubst %.o,../%.cpp,$(TOBJS))
		 
SLICE_SRCS	= $(SDIR)/Test.ice \
		  $(SDIR)/ClientPrivate.ice \
		  $(SDIR)/Forward.ice

SDIR		= ..

include $(top_srcdir)/config/Make.rules

SLICE2CPPEFLAGS	:= -I.. $(SLICE2CPPEFLAGS)
CPPFLAGS	:= -I. -I.. -DICEE_PURE_CLIENT -I../../../../include $(CPPFLAGS)
ifdef ice_bin_dist
LDFLAGS		:= $(LDFLAGS) -L$(libdir)
endif

$(CLIENT): $(OBJS) $(TOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(TOBJS) $(TESTCLIBS)

include .depend
