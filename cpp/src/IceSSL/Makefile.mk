#
# IceSSL lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceSSL
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_CPPFLAGS                  = -Icpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice --include-dir IceSSL --dll-export ICE_SSL_API 
LOCAL_LINKWITH                  = -Wl,-Bdynamic -lssl -lcrypto
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(DYNAMICLIBRARY_RULES)
