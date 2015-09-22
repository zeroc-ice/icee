# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= client

TARGETS		= $(CLIENT)

OBJS		= Client.o \
		  Latency.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Latency.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -DICEE_PURE_CLIENT $(CPPFLAGS)

$(CLIENT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(MINLIBS)

include .depend
