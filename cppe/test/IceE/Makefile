# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

SUBDIRS		= adapterDeactivation \
		  binding \
		  custom \
		  defaultServant \
		  exceptions \
		  facets \
		  faultTolerance \
		  inheritance \
		  interceptor \
		  location \
		  objects \
		  operations \
	 	  proxy \
		  retry \
		  slicing \
		  thread \
		  priority \
		  threadPoolPriority \
		  timeout \
		  uuid

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done
