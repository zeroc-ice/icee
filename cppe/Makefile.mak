# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

!include $(top_srcdir)/config/Make.rules.mak

SUBDIRS		= src include test demo

INSTALL_SUBDIRS	= $(install_bindir) $(install_libdir) $(install_includedir)

install:: install-common
	@for %i in ( $(INSTALL_SUBDIRS) ) do \
	    @if not exist %i \
		@echo "Creating %i..." && \
		mkdir %i
	copy $(ice_cpp_dir)\bin\slice2cppe.exe $(install_bindir)

$(EVERYTHING)::
	@if not exist include\IceE\Features.h \
	    @echo "*** You must run nmake /f Makefile.mak configure first." && exit 1
	@for %i in ( $(SUBDIRS) ) do \
	    @echo "making $@ in %i" && \
	    cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

test::
	@python $(top_srcdir)/allTests.py
