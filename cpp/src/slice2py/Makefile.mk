#
# slice2cpp
#

include $(CLEAR_RULES)
LOCAL_MODULE                    = slice2py
LOCAL_PATH                      = cpp/src/slice2py

LOCAL_SLICEDIR                  = ice/$(LOCAL_PATH)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SRCDIR)/*.ice)

LOCAL_DEPENDENT_MODULES         = Slice Ice mcpp
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_CPPFLAGS                  = -I$(LOCAL_PATH)

include $(APPLICATION_RULES)
