# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

SERVER		= ..\server.exe
COLLOCATED	= ..\collocated.exe

TARGETS		= $(SERVER) $(COLLOCATED)

OBJS		= Test.obj

TOBJS		= TestI.obj

SOBJS		= Server.obj

COBJS		= Collocated.obj \
		  AllTests.obj \
		  MyObjectFactory.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  ..\TestI.cpp \
		  ..\Server.cpp \
		  ..\Collocated.cpp \
		  ..\AllTests.cpp \
		  ..\MyObjectFactory.cpp

SDIR		= ..

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I.. -I../../../include $(CPPFLAGS) -WX -DWIN32_LEAN_AND_MEAN
!if "$(ice_bin_dist)" != ""
LDFLAGS		= $(LDFLAGS) /LIBPATH:"$(libdir)"
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
CPDBFLAGS        = /pdb:$(COLLOCATED:.exe=.pdb)
!endif

$(SERVER): $(OBJS) $(TOBJS) $(SOBJS)
	$(LINK) $(LDFLAGS) $(SPDBFLAGS) $(OBJS) $(TOBJS) $(SOBJS) /out:$@ $(TESTLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

$(COLLOCATED): $(OBJS) $(TOBJS) $(COBJS)
	$(LINK) $(LDFLAGS) $(CPDBFLAGS) $(OBJS) $(TOBJS) $(COBJS) /out:$@ $(TESTLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h

!include .depend
