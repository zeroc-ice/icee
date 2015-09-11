#
# IceUtil lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceUtil
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_LINKWITH                  = -Wl,-Bdynamic -lcrypt
LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(DYNAMICLIBRARY_RULES)
