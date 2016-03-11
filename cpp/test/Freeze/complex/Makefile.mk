# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

ifeq ($(BUILD_TESTSUITE),dynamic)
    include $(CLEAR_RULES)
    LOCAL_EXE                   = client
    LOCAL_PATH                  = cpp/test/Freeze/complex
    LOCAL_SLICES                = Complex.ice
    LOCAL_SRCS                  = Grammar.cpp Scanner.cpp Parser.cpp Client.cpp
    LOCAL_RESOURCES             = db
    LOCAL_DEPENDENT_MODULES     = Freeze

    LOCAL_FREEZEOUTPUTDIR       = $(LOCAL_PATH)
    LOCAL_SLICE2FREEZEFLAGS     = -I$(LOCAL_PATH) -Iice/$(LOCAL_PATH)
    LOCAL_FREEZE_CLASS          = ComplexDict
    LOCAL_FREEZE_DICT           = Complex::ComplexDict,Complex::Key,Complex::Node ComplexDict ice/$(LOCAL_PATH)/Complex.ice
    LOCAL_FREEZE_DEPS           = ice/$(LOCAL_PATH)/Complex.ice
    include $(FREEZE_RULES)

    include $(TEST_APPLICATION_RULES)
endif
