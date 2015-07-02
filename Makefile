# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#
prefix			?= /opt/IceE-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with
# optimization. Otherwise Ice is build with debug information.
#
#OPTIMIZE		= yes

#
# Debian arch tuple of the host machine supported values are 
# arm-linux-gnueabihf and x86_64-linux-gnu.
#
# Set it to arm-linux-gnueabihf if you are cross compiling for
# armhf.
#
HOST			?= arm-linux-gnueabihf

#
# If cross compiling set it to the root directory of your
# cross development third party libraries.
#
CROSS_HOME  		?= /opt/IceE-3.6.0-ThirdParty/armhf


#
# Include common definitions
#
include $(CURDIR)/config/Make.common.rules

CLEAR_RULES 		= $(CURDIR)/config/Make.clear.rules
OBJECT_RULES 		= $(CURDIR)/config/Make.object.rules
SUBMODULE_RULES		= $(CURDIR)/config/Make.submodule.rules
APPLICATION_RULES 	= $(CURDIR)/config/Make.application.rules
TEST_APPLICATION_RULES 	= $(CURDIR)/config/Make.test.application.rules
CLIENTSERVERTEST_RULES	= $(CURDIR)/config/Make.clientservertest.rules
STATICLIBRARY_RULES 	= $(CURDIR)/config/Make.staticlibrary.rules
DYNAMICLIBRARY_RULES 	= $(CURDIR)/config/Make.dynamiclibrary.rules

#
# Include all .mk files
#
include $(call rwildcard,,*.mk)

dist:: Ice Glacier2 IceBox IceGrid IceStorm glacier2router icebox iceboxadmin 

all:: $(TARGETS)

clean:: $(CLEAN_TARGETS)

.PHONY: $(CLEAN_TARGETS) deploy_test

test_deploy:
	rsync -aczv --exclude="*.d" --exclude="*.h" --exclude="*.cpp" --exclude="*.mk" --exclude="*.o" --prune-empty-dirs cpp debian@192.168.7.2:/home/debian
	rsync -aczv --include="*/test/Ice/" --include="*/test/IceUtil/" --include="*.py" --exclude="*" --prune-empty-dirs ice/cpp debian@192.168.7.2:/home/debian
	rsync -aczv --include="*/" --include="*.py" --exclude="*" --prune-empty-dirs ice/scripts debian@192.168.7.2:/home/debian

#
# Include depend rules
#
include $(CURDIR)/config/Make.depend.rules