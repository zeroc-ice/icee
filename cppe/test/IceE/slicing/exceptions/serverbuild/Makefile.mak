# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..\..

SERVER		= ..\server.exe

TARGETS		= $(SERVER)

OBJS		= Test.obj \
		  ServerPrivate.obj

SOBJS		= TestI.obj \
		  Server.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  ..\TestI.cpp \
		  ..\Server.cpp

SDIR		= ..

!include $(top_srcdir)/config/Make.rules.mak

SLICE2CPPEFLAGS	= -I.. $(SLICE2CPPEFLAGS)
CPPFLAGS	= -I. -I.. -I../../../../include $(CPPFLAGS) -WX -DWIN32_LEAN_AND_MEAN
!if "$(ice_bin_dist)" != ""
LDFLAGS		= $(LDFLAGS) /LIBPATH:"$(libdir)"
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LDFLAGS) $(PDBFLAGS) $(OBJS) $(SOBJS) /out:$@ $(TESTLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h

!include .depend
