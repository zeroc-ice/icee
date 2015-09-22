# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#
prefix			= C:\IceE-$(VERSION)

#
# Define OPTIMIZE_SIZE as yes if you want to build with minimal size.
# Define OPTIMIZE_SPEED as yes if you want to build with maximum speed.
# These options are mutually exclusive.
# If neither is set, IceE is built with debug information.
#
#OPTIMIZE_SIZE		= yes
#OPTIMIZE_SPEED		= yes

#
# Define STATICLIBS as yes if you want to build with static libraries.
# Otherwise IceE is built with dynamic libraries. If you want the cpp
# runtime linked statically as well set STATIC_CPP_RUNTIME to yes.
#
#STATICLIBS             = yes
#STATIC_CPP_RUNTIME	= yes

#
# Define WINDOWS_MOBILE_SDK as yes if building for Windows Mobile 6
# Professional.
#
#WINDOWS_MOBILE_SDK	= yes

#
# Change the following setting if the Windows Mobile 6 SDK is installed
# in a different location than the default. For example, on Windows x64
# the SDK is installed in "C:\Program Files (x86)" by default.
#
WMSDK_BASE_DIR		= C:\Program Files\Windows Mobile 6 SDK

# ----------------------------------------------------------------------
# Ice-E supports a number of optional features that are enabled via
# build-time settings. To minimize the size of your executables, review
# the features below and disable any setting that is not required by
# your application.
#
# Prior to your first build, or after any subsequent change to the
# settings below, you must run 'nmake /f Makefile.mak configure' to
# generate the header file include\IceE\Features.h.
# ----------------------------------------------------------------------

#
# Compile with support for the Ice router facility.
#
!ifndef HAS_ROUTER
HAS_ROUTER		= yes
!endif

#
# Compile with support for the Ice locator facility.
#
!ifndef HAS_LOCATOR
HAS_LOCATOR		= yes
!endif

#
# Compile with support for batch invocations.
#
!ifndef HAS_BATCH
HAS_BATCH		= yes
!endif

#
# Compile with support for wstring and string conversion.
#
!ifndef HAS_WSTRING
HAS_WSTRING		= yes
!endif

#
# Compile with support for opaque endpoints.
#
!ifndef HAS_OPAQUE_ENDPOINTS
HAS_OPAQUE_ENDPOINTS	= yes
!endif

#
# Compile with support for asynchronous method invocation (AMI).
#
!ifndef HAS_AMI
HAS_AMI			= yes
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = cppe
slice_translator = slice2cppe.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak.icee)
!include $(top_srcdir)\..\config\Make.common.rules.mak.icee
!else
!include $(top_srcdir)\config\Make.common.rules.mak.icee
!endif

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
headerdir		= $(top_srcdir)\include

!if "$(ice_src_dist)" != ""
includedir		= $(top_srcdir)\include
!else
includedir		= $(ice_dir)\include
!endif

install_bindir		= $(prefix)\bin$(x64suffix)
install_libdir	  	= $(prefix)\lib$(x64suffix)
install_includedir	= $(prefix)\include


#
# Set executables
#
MT		= mt.exe
RC		= rc.exe
!if "$(WINDOWS_MOBILE_SDK)" == "yes"
CXX		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\cl.exe"
CC		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\cl.exe"
LINK 		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\link.exe"
AR		= "$(VSINSTALLDIR)\VC\ce\bin\x86_arm\lib.exe"
RC		= $(RC) /i "$(VSINSTALLDIR)\VC\ce\atlmfc\include"
!else
CXX		= cl.exe
CC		= cl.exe
LINK 		= link.exe
AR		= lib.exe
!endif


CPPFLAGS	= -nologo -W3 -GR -EHsc -FD -D_CONSOLE -I$(includedir)

#
# Add options for WinCE support
#
!if "$(WINDOWS_MOBILE_SDK)" == "yes"

SDK_DIR		= $(WMSDK_BASE_DIR)\PocketPC

#
# Windows Mobile 6 is based on Windows CE 5.2
# Windows CE is a Unicode operating system: http://msdn.microsoft.com/en-us/library/ms904336.aspx
#

RC		= $(RC) /d "UNDER_CE=0x502" /d "_WIN32_WCE=0x502" -I"$(SDK_DIR)\Include\ArmV4i"
CPPFLAGS 	= -QRarch4 -I"$(VSINSTALLDIR)\VC\ce\Include" -I"$(SDK_DIR)\Include\ArmV4i" -fp:fast -TP $(CPPFLAGS) /D "_WIN32_WCE=0x502" /D "UNDER_CE=0x502" /D "ARM" /D "_ARM_" /D "ARMV4" /D "_UNICODE" /D "UNICODE"

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
CPPFLAGS	= $(CPPFLAGS) -GS-
!endif

LDFLAGS		= /LIBPATH:"$(VSINSTALLDIR)\VC\ce\Lib\ArmV4i" /LIBPATH:"$(SDK_DIR)\Lib\ArmV4i" -nodefaultlib:"kernel32.lib" -nodefaultlib:"oldnames.lib" /STACK:65536,4096 /MACHINE:THUMB


CPPFLAGS	= $(CPPFLAGS) /D "WIN32_PLATFORM_PSPC"

!endif

#
# Add options for static library support
#
!if "$(STATICLIBS)" == "yes"
CPPFLAGS	= $(CPPFLAGS) -DICEE_STATIC_LIBS
!endif

#
# Add release vs debug options
#
!if "$(OPTIMIZE_SPEED)" == "yes" || "$(OPTIMIZE_SIZE)" == "yes"

CPPFLAGS	= $(CPPFLAGS) -DNDEBUG -GL

!if "$(STATICLIBS)" == "yes" && "$(STATIC_CPP_RUNTIME)" == "yes"
CPPFLAGS        = $(CPPFLAGS) -MT
!else
CPPFLAGS        = $(CPPFLAGS) -MD
!endif

!if "$(OPTIMIZE_SPEED)" == "yes"
CPPFLAGS	= $(CPPFLAGS) -O2
!else
CPPFLAGS        = $(CPPFLAGS) -O1
!endif

!else # Debug

CPPFLAGS	= $(CPPFLAGS) -Zi -Gm -Od -D_DEBUG

!if "$(STATICLIBS)" == "yes" && "$(STATIC_CPP_RUNTIME)" == "yes"
CPPFLAGS        = $(CPPFLAGS) -MTd
!else
CPPFLAGS        = $(CPPFLAGS) -MDd
!endif

!if "$(WINDOWS_MOBILE_SDK)" != "yes"
CPPFLAGS        = $(CPPFLAGS) -RTC1
!endif

!endif

#
# Linker flags
#
!if "$(ice_src_dist)" != ""
LDFLAGS         = $(LDFLAGS) /LIBPATH:"$(libdir)" /nologo
!else
LDFLAGS         = $(LDFLAGS) /LIBPATH:"$(ice_dir)\lib$(x64suffix)" /nologo
!endif

!if "$(WINDOWS_MOBILE_SDK)" == "yes"
LDFLAGS		= $(LDFLAGS) -manifest:no /subsystem:windowsce
!endif

!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
LDFLAGS		= $(LDFLAGS) /debug /fixed:no /incremental:yes
!else
LDFLAGS         = $(LDFLAGS) /OPT:REF /incremental:no /LTCG
!endif

ARFLAGS		= /nologo
!if "$(OPTIMIZE_SPEED)" == "yes" || "$(OPTIMIZE_SIZE)" == "yes"
ARFLAGS		= $(ARFLAGS) /LTCG
!endif

#
# MFC specific flags
#

!if "$(WINDOWS_MOBILE_SDK)" == "yes"

MFC_CPPFLAGS	= -I"$(VSINSTALLDIR)\VC\ce\atlmfc\include" 
MFC_LDFLAGS	= /LIBPATH:"$(VSINSTALLDIR)\VC\ce\atlmfc\lib\ArmV4i"

!else

MFC_LDFLAGS     = /subsystem:windows

!endif

#
# Set up libraries
#
!if "$(STATICLIBS)" == "yes"
LIBSUFFIX	= _static$(LIBSUFFIX)
!endif
!if "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif


!if "$(WINDOWS_MOBILE_SDK)" == "yes"

BASELIBS	= coredll.lib ws2.lib corelibc.lib

!else

BASELIBS        = rpcrt4.lib advapi32.lib ws2_32.lib
!if "$(STATICLIBS)" == "yes" && "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
BASELIBS	= $(BASELIBS)
!endif

!endif

LIBS		= icee$(LIBSUFFIX).lib
MINLIBS		= iceec$(LIBSUFFIX).lib

!if "$(STATICLIBS)" == "yes" || "$(WINDOWS_MOBILE_SDK)" == "yes"
LIBS		= $(LIBS) $(BASELIBS)
MINLIBS		= $(MINLIBS) $(BASELIBS)
!endif

MFC_LIBS	= $(LIBS)
MFC_MINLIBS	= $(MINLIBS)
!if "$(WINDOWS_MOBILE_SDK)" == "" && "$(STATICLIBS)" == "yes" && "$(STATIC_CPP_RUNTIME)" == "yes"
!if  "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes"
MFC_LIBS        = nafxcwd.lib $(LIBS)
MFC_MINLIBS	= nafxcwd.lib $(MINLIBS)
!else
MFC_LIBS        = nafxcw.lib $(LIBS)
MFC_MINLIBS	= nafxcw.lib $(MINLIBS)
!endif
!endif

TESTLIBS	= testcommon$(LIBSUFFIX).lib $(LIBS)
TESTCLIBS	= testcommonc$(LIBSUFFIX).lib $(MINLIBS)

SLICE2CPPEFLAGS		= -I$(slicedir)

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2CPPE		= "$(ice_cpp_dir)\bin\slice2cppe.exe"
!else
SLICE2CPPE		= "$(ice_cpp_dir)\bin$(x64suffix)\slice2cppe.exe"
!endif
!else
SLICE2CPPE		= "$(ice_dir)\bin$(x64suffix)\slice2cppe.exe"
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj .cobj

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

{..\}.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<

!if "$(HDIR)" != ""

{$(SDIR)\}.ice{$(HDIR)}.h:
	del /q $(HDIR)\$(*F).h $(*F).cpp
	$(SLICE2CPPE) $(SLICE2CPPEFLAGS) $<
	move $(*F).h $(HDIR)

!else

{$(SDIR)\}.ice.h:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPPE) $(SLICE2CPPEFLAGS) $<

!endif

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPPE) $(SLICE2CPPEFLAGS) $(*F).ice

all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

clean::
	-del /q *.obj *.cobj *.bak *.ilk *.exp *.pdb

install::

#
# Generate Features.h using the settings enabled above.
#
configure:
	$(top_srcdir)\config\features.bat $(top_srcdir)\include\IceE\Features.h \
	    "HAS_ROUTER=$(HAS_ROUTER)" \
	    "HAS_LOCATOR=$(HAS_LOCATOR)" \
	    "HAS_BATCH=$(HAS_BATCH)" \
	    "HAS_WSTRING=$(HAS_WSTRING)" \
	    "HAS_OPAQUE_ENDPOINTS=$(HAS_OPAQUE_ENDPOINTS)" \
	    "HAS_AMI=$(HAS_AMI)"

