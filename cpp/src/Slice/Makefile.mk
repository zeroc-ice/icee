#
# Slice lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = Slice
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)
LOCAL_DEPENDENT_MODULES         = mcpp
LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS                  = -Icpp/src/$(LOCAL_MODULE)
LOCAL_OBJS                      = $(LIBRARY_OBJS)
LOCAL_PUBLIC_HEADERS            = $(wildcard ice/cpp/include/$(LOCAL_MODULE)/*.h)
LOCAL_DEPENDENT_DYMODULES       = IceUtil
include $(LIBRARY_RULES)

Slice: Slice_dynamiclib