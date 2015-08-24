#
# IceUtil lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceUtil
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS                  = -Icpp/src/$(LOCAL_MODULE)
LOCAL_LINKWITH                  = -Wl,-Bdynamic -lcrypt
LOCAL_PUBLIC_HEADERS            = $(wildcard ice/cpp/include/$(LOCAL_MODULE)/*.h)

include $(DYNAMICLIBRARY_RULES)
include $(INSTALL_RULES)