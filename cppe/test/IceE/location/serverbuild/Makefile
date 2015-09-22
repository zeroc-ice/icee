# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

SERVER		= ../server

TARGETS		= $(SERVER)

OBJS		= Test.o

SOBJS		= TestI.o \
		  Server.o \
		  ServerLocator.o
	
SRCS		= $(OBJS:.o=.cpp) \
		  $(patsubst %.o,../%.cpp,$(SOBJS))

SLICE_SRCS	= $(SDIR)/Test.ice

SDIR		= ..

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../../include $(CPPFLAGS)
ifdef ice_bin_dist
LDFLAGS		:= $(LDFLAGS) -L$(libdir)
endif

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(TESTLIBS)

include .depend
