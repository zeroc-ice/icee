#
# Glacier2 lib
#
include $(CLEAR_RULES)

LOCAL_MODULE	= Glacier2
LOCAL_PATH	= cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR	= ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES	= $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS	= --ice -Iice/slice --dll-export GLACIER2_API

LOCAL_SRCDIR	= ice/cpp/src/$(LOCAL_MODULE)Lib
LOCAL_SRCS 	= $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS	= -Icpp/src/$(LOCAL_MODULE)

include $(STATICLIBRARY_RULES)

#
# Glacier2CryptPermissionsVerifier
#
include $(CLEAR_RULES)
LOCAL_MODULE		= Glacier2CryptPermissionsVerifier
include $(SUBMODULE_RULES)
SUBMODULE_OBJS := $(LOCAL_OBJS)

include $(CLEAR_RULES)
LOCAL_MODULE	= glacier2router
LOCAL_EXE	= $(LOCAL_MODULE)
LOCAL_PATH	= cpp/src/Glacier2

LOCAL_SLICEDIR	= ice/$(LOCAL_PATH)
LOCAL_SLICES	= $(wildcard $(LOCAL_SRCDIR)/*.ice)

LOCAL_LINK_WITH	= Glacier2 Ice
LOCAL_SRCDIR	= ice/$(LOCAL_PATH)
LOCAL_SRCS 	= $(wildcard $(LOCAL_SRCDIR)/*.cpp) \
		  $(LOCAL_PATH)/RegisterPluginsInit.cpp

LOCAL_CPPFLAGS	= -I$(LOCAL_PATH)
LOCAL_OBJS	= $(SUBMODULE_OBJS)
LOCAL_LDFLAGS	= -lcrypt

include $(APPLICATION_RULES)

SUBMODULE_OBJS	=
