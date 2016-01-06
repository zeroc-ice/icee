#
# IceXML lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceXML
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_LINKWITH                  = -Wl,-Bdynamic -lexpat

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)
LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

include $(DYNAMICLIBRARY_RULES)