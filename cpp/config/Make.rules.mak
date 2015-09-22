# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Define OPTIMIZE as yes if you want to build with
# optimization. Otherwise build with debug information.
#
OPTIMIZE		= yes

#
# Specify your C++ compiler. Supported values are:
# VC80 and VC90
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER		= VC80
!endif

#
# If Mcpp is not installed in a standard location where the  compiler 
# can find it, set MCPP_HOME to the Mcpp installation directory.
#
!if "$(MCPP_HOME)" == ""
MCPP_HOME		= C:\Ice-3.3.0-ThirdParty-$(CPP_COMPILER)
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

STATICLIBS              = yes
!if "$(STATIC_CPP_RUNTIME)" == ""
STATIC_CPP_RUNTIME	= no
!endif

#
# Common definitions
#
ice_language     = cpp
!include $(top_srcdir)\..\config\Make.common.rules.mak.icee

#
# Platform specific definitions
#
!include $(top_srcdir)\config\Make.rules.msvc

VERSION			= 1.3.0

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
includedir		= $(top_srcdir)\include

install_bindir		= $(prefix)\bin

SETARGV			= setargv.obj
MT			= mt.exe

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif

!if "$(MCPP_HOME)" != ""
CPPFLAGS        = -I"$(MCPP_HOME)\include" $(CPPFLAGS)
LDFLAGS         = $(PRELIBPATH)"$(MCPP_HOME)\lib$(x64suffix)" $(LDFLAGS)
!endif

CPPFLAGS		= $(CPPFLAGS) -I$(includedir)
LDFLAGS			= $(LDFLAGS) $(PRELIBPATH)"$(libdir)" $(LDPLATFORMFLAGS) $(CXXFLAGS)

SLICEPARSERLIB		= $(libdir)\slice$(LIBSUFFIX).lib

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj .res .rc

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<

.rc.res:
	rc $(RCFLAGS) $<

all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

clean::
	-del /q *.obj *.bak *.ilk *.exp *.pdb *.tds

install::
