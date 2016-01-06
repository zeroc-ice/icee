#
# IceBT lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceBT
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice --include-dir IceBT --dll-export ICE_BT_API

LOCAL_CPPFLAGS                  = `pkg-config --cflags dbus-1`
LOCAL_LINKWITH                  = -Wl,-Bdynamic `pkg-config --libs dbus-1`
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil IceXML

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(DYNAMICLIBRARY_RULES)
