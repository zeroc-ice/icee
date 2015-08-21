#
# mcpp
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = mcpp
LOCAL_SRCDIR                    = mcpp
LOCAL_PATH                      = mcpp
LOCAL_C_SRCS                    = $(filter-out mcpp/main.c, $(wildcard $(LOCAL_SRCDIR)/*.c)) 
include $(OBJECT_RULES)
LIBRARY_OBJS                    := $(LOCAL_OBJS)
#
# Slice lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = Slice
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS                  = -Icpp/src/$(LOCAL_MODULE)
LOCAL_OBJS                      = $(LIBRARY_OBJS)
LOCAL_PUBLIC_HEADERS            = $(wildcard ice/cpp/include/$(LOCAL_MODULE)/*.h)

include $(STATICLIBRARY_RULES)

LIBRARY_OBJS			=