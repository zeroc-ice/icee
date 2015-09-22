# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,TestCommon,$(VERSION))
SONAME          = $(call mksoname,TestCommon,$(SOVERSION))  
LIBNAME		= $(call mklibname,TestCommon)

LIBFILENAMEC    = $(call mklibfilename,TestCommonC,$(VERSION))
SONAMEC         = $(call mksoname,TestCommonC,$(SOVERSION))  
LIBNAMEC	= $(call mklibname,TestCommonC)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME)) \
		  $(call mklibtargets,$(libdir)/$(LIBFILENAMEC),$(libdir)/$(SONAMEC),$(libdir)/$(LIBNAMEC))

OBJS		= TestCommon.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I../include $(CPPFLAGS) -DICE_TEST_COMMON_API_EXPORTS
LINKWITH	:= -lIceE
LINKWITHC	:= -lIceEC

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))

$(libdir)/$(LIBNAMEC): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(libdir)/$(LIBFILENAMEC): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAMEC),$(OBJS),$(LINKWITHC))

$(libdir)/$(SONAMEC): $(libdir)/$(LIBFILENAMEC)
	rm -f $@
	ln -s $(LIBFILENAMEC) $@

$(libdir)/$(LIBNAMEC): $(libdir)/$(SONAMEC)
	rm -f $@
	ln -s $(SONAMEC) $@
endif

include .depend
