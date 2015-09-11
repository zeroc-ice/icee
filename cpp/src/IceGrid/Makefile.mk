#
# IceGrid
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceGrid
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice --include-dir IceGrid --dll-export ICE_GRID_API 

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)Lib
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_DEPENDENT_DYMODULES       = Glacier2 Ice IceUtil
LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(LIBRARY_RULES)

