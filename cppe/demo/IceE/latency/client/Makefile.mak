# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= client.exe

TARGETS		= $(CLIENT)

OBJS		= Latency.obj

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINDOWS_MOBILE_SDK)" == "yes"
COBJS           = WinCEClient.obj
!else
COBJS           = Client.obj
!endif

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp)

CPPFLAGS	= -I. -DICEE_PURE_CLIENT $(CPPFLAGS) -WX -DWIN32_LEAN_AND_MEAN

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LDFLAGS) $(PDBFLAGS) $(OBJS) $(COBJS) /out:$@ $(MINLIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest

clean::
	del /q Latency.cpp Latency.h

!include .depend
