#
# IceDiscovery lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceDiscovery
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice --include-dir IceDiscovery
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil
LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(DYNAMICLIBRARY_RULES)

IceDiscovery: IceDiscovery_dynamiclib