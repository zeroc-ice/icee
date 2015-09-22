# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\testcommon$(LIBSUFFIX).lib
CLIBNAME	= $(top_srcdir)\lib\testcommonc$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\testcommon$(SOVERSION)$(LIBSUFFIX).dll
CDLLNAME	= $(top_srcdir)\bin\testcommonc$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME) $(CLIBNAME) $(CDLLNAME)

OBJS  		= TestCommon.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I../include $(CPPFLAGS) -DICE_TEST_COMMON_API_EXPORTS -DWIN32_LEAN_AND_MEAN

!if "$(STATICLIBS)" != "yes" && "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

!if "$(STATICLIBS)" == "yes"

$(DLLNAME):

$(LIBNAME): $(OBJS)
        $(AR) $(ARFLAGS) $(PDBFLAGS) $(OBJS) /out:$(LIBNAME)

$(CDLLNAME):

$(CLIBNAME): $(OBJS)
        $(AR) $(ARFLAGS) $(PDBFLAGS) $(OBJS) /out:$(CLIBNAME)

!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LDFLAGS) /dll $(PDBFLAGS) $(OBJS) /out:$(DLLNAME) $(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(CLIBNAME): $(CDLLNAME)

$(CDLLNAME): $(OBJS)
	$(LINK) $(LDFLAGS) /dll $(PDBFLAGS) $(OBJS) /out:$(CDLLNAME) $(MINLIBS)
	move $(CDLLNAME:.dll=.lib) $(CLIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(CDLLNAME:.dll=.exp) del /q $(CDLLNAME:.dll=.exp)

!endif

clean::
	del /q $(LIBNAME:.dll=.*)
	del /q $(CLIBNAME:.dll=.*)

!if "$(STATICLIBS)" != "yes"

clean::
	del /q $(DLLNAME:.dll=.*)
	del /q $(CDLLNAME:.dll=.*)

!endif

!include .depend
