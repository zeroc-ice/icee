#
# Glacier2 lib
#
include $(CLEAR_RULES)

LOCAL_MODULE	= IceBox
LOCAL_PATH	= cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR	= ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES	= $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS	= --ice -Iice/slice --dll-export ICE_BOX_API

LOCAL_SRCDIR	= ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS 	= $(LOCAL_SRCDIR)/Exception.cpp
LOCAL_CPPFLAGS	= -Icpp/src/$(LOCAL_MODULE)

include $(STATICLIBRARY_RULES)
