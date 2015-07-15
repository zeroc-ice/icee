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
# Define OPTIMIZE as yes if you want to build with optimization. Otherwise
# Ice-E is built with debug information.
#
#OPTIMIZE		= yes

#
# Debian arch tuple of the host machine. Supported values are
# arm-linux-gnueabihf and x86_64-linux-gnu.
#
# Set it to arm-linux-gnueabihf if you are cross-compiling for
# armhf.
#
HOST			?= arm-linux-gnueabihf

#
# If you want to build Ice-E using an Ice binary distribution
# set ICE_HOME to point to your Ice install location.
#
#ICE_HOME		= /usr

#
# If cross-compiling, set this to the root directory of your
# cross development third party libraries.
#
THIRDPARTY_HOME  	?= /opt/IceE-3.6.0-ThirdParty

#
# Target to deploy the test suite.
#
DEPLOY_TARGET		?= debian@192.168.7.2:/home/debian/icee

#
# Include common definitions
#
include $(CURDIR)/config/Make.common.rules
