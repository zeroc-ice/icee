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
    LOCAL_PATH                  = cpp/test/Freeze/dbmap
    LOCAL_SRCS                  = Client.cpp
    LOCAL_RESOURCES             = db
    LOCAL_DEPENDENT_MODULES     = Freeze

    LOCAL_FREEZEOUTPUTDIR       = $(LOCAL_PATH)
    LOCAL_SLICE2FREEZEFLAGS     = -I$(LOCAL_PATH) --ice $(LOCAL_SLICE2CPPFLAGS)

    LOCAL_FREEZE_CLASS          = ByteIntMap
    LOCAL_FREEZE_DICT           = Test::ByteIntMap,byte,int
    LOCAL_FREEZE_DICT_INDEX     = "Test::ByteIntMap,sort" ByteIntMap
    include $(FREEZE_RULES)

    LOCAL_FREEZE_CLASS          = IntIdentityMap
    LOCAL_FREEZE_DICT           = Test::IntIdentityMap,int,Ice::Identity IntIdentityMap ice/slice/Ice/Identity.ice
    LOCAL_FREEZE_DEPS           = ice/slice/Ice/Identity.ice
    include $(FREEZE_RULES)

    LOCAL_FREEZE_CLASS          = IntIdentityMapWithIndex
    LOCAL_FREEZE_DICT           = Test::IntIdentityMapWithIndex,int,Ice::Identity
    LOCAL_FREEZE_DICT_INDEX     = Test::IntIdentityMapWithIndex,category IntIdentityMapWithIndex ice/slice/Ice/Identity.ice
    LOCAL_FREEZE_DEPS           = ice/slice/Ice/Identity.ice
    include $(FREEZE_RULES)

    LOCAL_FREEZE_CLASS          = SortedMap
    LOCAL_FREEZE_DICT           = Test::SortedMap,int,Ice::Identity,sort SortedMap
    LOCAL_FREEZE_DICT_INDEX     = "Test::SortedMap,category,sort,std::greater<std::string>" ice/slice/Ice/Identity.ice
    LOCAL_FREEZE_DEPS           = ice/slice/Ice/Identity.ice
    include $(FREEZE_RULES)

    LOCAL_FREEZE_CLASS          = WstringWstringMap
    LOCAL_FREEZE_DICT           = 'Test::WstringWstringMap,["cpp:type:wstring"]string,["cpp:type:wstring"]string'
    LOCAL_FREEZE_DICT_INDEX     = Test::WstringWstringMap WstringWstringMap
    include $(FREEZE_RULES)

    include $(TEST_APPLICATION_RULES)
endif
