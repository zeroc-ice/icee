# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= src include test demo

INSTALL_SUBDIRS = $(install_bindir) $(install_libdir) $(install_includedir)

install:: install-common
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    if test ! -d $$subdir ; \
	    then \
	        echo "Creating $$subdir..." ; \
	        mkdir -p $$subdir ; \
	        chmod a+rx $$subdir ; \
	    fi ; \
	done
	$(call installprogram,$(ice_cpp_dir)/bin/slice2cppe,$(install_bindir))

$(EVERYTHING)::
	@if [ ! -f include/IceE/Features.h ]; \
	then \
	    echo "*** You must run make configure first."; \
	    exit 1; \
        fi
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

test::
	@python $(top_srcdir)/allTests.py
