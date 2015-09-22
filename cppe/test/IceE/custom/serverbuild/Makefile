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
COLLOCATED	= ../collocated

TARGETS		= $(SERVER) $(COLLOCATED)

OBJS		= Test.o \
		  Wstring.o

TOBJS		= TestI.o \
		  WstringI.o \
		  MyByteSeq.o \
		  StringConverterI.o

SOBJS		= Server.o

COBJS		= Collocated.o \
		  AllTests.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(patsubst %.o,../%.cpp,$(TOBJS)) \
		  $(patsubst %.o,../%.cpp,$(SOBJS)) \
		  $(patsubst %.o,../%.cpp,$(COBJS))

SLICE_SRCS	= $(SDIR)/Test.ice \
		  $(SDIR)/Wstring.ice

SDIR		= ..

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../../include $(CPPFLAGS)
ifdef ice_bin_dist
LDFLAGS		:= $(LDFLAGS) -L$(libdir)
endif

$(SERVER): $(OBJS) $(TOBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(TOBJS) $(SOBJS) $(TESTLIBS)

$(COLLOCATED): $(OBJS) $(TOBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(TOBJS) $(COBJS) $(TESTLIBS)

include .depend
