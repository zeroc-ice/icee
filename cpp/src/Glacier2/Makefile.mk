#
# Glacier2 lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = Glacier2
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice --dll-export GLACIER2_API --include-dir Glacier2

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)Lib
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil

include $(LIBRARY_RULES)

ifneq ($(CPP11),yes)

include $(CLEAR_RULES)
LOCAL_MODULE                    = glacier2router
LOCAL_PATH                      = cpp/src/Glacier2

LOCAL_SLICEDIR                  = ice/$(LOCAL_PATH)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --include-dir Glacier2

LOCAL_DEPENDENT_DYMODULES       = Glacier2CryptPermissionsVerifier Glacier2 IceSSL Ice IceUtil
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_LINKWITH                  = -Wl,-Bdynamic -lssl
LOCAL_LDFLAGS                   = -rdynamic
include $(APPLICATION_RULES)

endif