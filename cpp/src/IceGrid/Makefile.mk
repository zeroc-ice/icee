#
# IceGrid
#
include $(CLEAR_RULES)

LOCAL_MODULE	= IceGrid
LOCAL_PATH	= cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR	= ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES	= $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS	= --ice -Iice/slice --dll-export ICE_GRID_API

LOCAL_SRCDIR	= ice/cpp/src/$(LOCAL_MODULE)Lib
LOCAL_SRCS 	= $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS	= -Icpp/src/$(LOCAL_MODULE)

include $(STATICLIBRARY_RULES)
