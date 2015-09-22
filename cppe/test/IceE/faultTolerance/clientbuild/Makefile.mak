# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= ..\client.exe

TARGETS		= $(CLIENT)

OBJS		= Test.obj

TOBJS		= Client.obj \
		  AllTests.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  ..\Client.cpp \
		  ..\AllTests.cpp

SDIR		= ..

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I.. -I../../../include -DICEE_PURE_CLIENT $(CPPFLAGS) -WX -DWIN32_LEAN_AND_MEAN
!if "$(ice_bin_dist)" != ""
LDFLAGS		= $(LDFLAGS) /LIBPATH:"$(libdir)"
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(TOBJS)
	$(LINK) $(LDFLAGS) $(PDBFLAGS) $(OBJS) $(TOBJS) /out:$@ $(TESTCLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

clean::
	del /q Test.cpp Test.h

!include .depend
