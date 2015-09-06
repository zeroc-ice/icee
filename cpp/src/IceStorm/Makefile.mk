#
# IceGrid
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceStorm
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/slice --include-dir IceStorm --dll-export ICE_STORM_API 

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)Lib
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS                  = -Icpp/src/$(LOCAL_MODULE)
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(LIBRARY_RULES)

