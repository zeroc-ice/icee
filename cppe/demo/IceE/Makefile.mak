# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)\config\Make.rules.mak

SUBDIRS		= minimal \
		  latency \
		  throughput \
		  chat \
		  MFC

!if "$(WINDOWS_MOBILE_SDK)" != "yes"
SUBDIRS 	= $(SUBDIRS) \
		  hello \
		  callback \
		  bidir \
		  nested \
		  value \
		  workqueue
!endif

!if "$(WINDOWS_MOBILE_SDK)" != "yes" && "$(HAS_AMI)" == "yes"
SUBDIRS		= $(SUBDIRS) async
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
