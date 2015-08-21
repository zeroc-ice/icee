#
# IceUtil
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = IceUtil
LOCAL_PUBLIC_HEADERS            = $(wildcard ice/cpp/include/$(LOCAL_MODULE)/*.h)
LOCAL_PUBLIC_SLICES             = $(wildcard ice/slice/$(LOCAL_MODULE)/*.ice)
include $(SUBMODULE_RULES)
LIBRARY_OBJS                    := $(LIBRARY_OBJS) $(LOCAL_OBJS)

#
# IceDiscovery
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = IceDiscovery
include $(SUBMODULE_RULES)
LIBRARY_OBJS                    := $(LIBRARY_OBJS) $(LOCAL_OBJS)

#
# IceLocatorDiscovery
#

include $(CLEAR_RULES)
LOCAL_MODULE                    = IceLocatorDiscovery
include $(SUBMODULE_RULES)
LIBRARY_OBJS                    := $(LIBRARY_OBJS) $(LOCAL_OBJS)

#
# IceSSL
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = IceSSL
LOCAL_PUBLIC_HEADERS            = $(wildcard ice/cpp/include/$(LOCAL_MODULE)/*.h)
LOCAL_PUBLIC_SLICES             = $(wildcard ice/slice/$(LOCAL_MODULE)/*.ice)
include $(SUBMODULE_RULES)
LIBRARY_OBJS                    := $(LIBRARY_OBJS) $(LOCAL_OBJS)

#
# Ice
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = Ice
LOCAL_PATH                      = cpp/src/Ice

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(filter-out $(LOCAL_SRCDIR)/DLLMain.cpp $(LOCAL_SRCDIR)/RegisterPluginsInit.cpp, $(wildcard $(LOCAL_SRCDIR)/*.cpp)) \
                                  $(LOCAL_PATH)/RegisterPluginsInit.cpp

LOCAL_CPPFLAGS                  = -Icpp/src/Ice

LOCAL_OBJS                      = $(LIBRARY_OBJS)

LOCAL_PUBLIC_HEADERS            = $(wildcard ice/cpp/include/$(LOCAL_MODULE)/*.h)
LOCAL_PUBLIC_SLICES             = $(LOCAL_SLICES)


include $(STATICLIBRARY_RULES)

include $(INSTALL_RULES)