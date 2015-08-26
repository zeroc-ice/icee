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
prefix                          ?= /opt/IceE-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with optimization. Otherwise
# Ice-E is built with debug information.
#
#OPTIMIZE                       = yes

#
# The target operating system for the build, supported values are 'yocto'
# and 'debian'.
#
ICEE_TARGET_OS                  ?= debian

#
# If you want to build Ice-E using an Ice binary distribution
# set ICE_HOME to point to your Ice install location.
#
#ICE_HOME                       = /usr

#
# Default Mutex protocol: one of PrioNone or PrioInherit.
#
#DEFAULT_MUTEX_PROTOCOL ?= PrioNone

#
# To build test suite against static libraries define BUILD_TESTSUITE
# as static, to build test suite against dynamic libraries define
# BUILD_TESTSUITE as dynamic.
#
BUILD_TESTSUITE                 ?= static

#
# Target to deploy the test suite.
#
DEPLOY_TARGET                   ?= debian@192.168.7.2:/home/debian/icee


###############################################################################
#                                                                             #
# Debian/Ubuntu specific options                                              #
#                                                                             #
###############################################################################

#
# Target distribution supported values are 'wheezy' 'trusty' and 'vivid'
#
TARGET_DIST                     ?= wheezy

# Debian arch tuple of the host machine. Supported values are
# arm-linux-gnueabihf and x86_64-linux-gnu.
#
# Set it to arm-linux-gnueabihf if you are cross-compiling for
# armhf.
#
HOST                            ?= arm-linux-gnueabihf

#
# If cross-compiling for debian , set this to the root directory
# of your cross development third party libraries.
#
THIRDPARTY_HOME                 ?= /opt/IceE-3.6.1-ThirdParty

#
# Include common definitions
#
include $(CURDIR)/config/Make.common.rules
