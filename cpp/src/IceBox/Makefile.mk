#
# IceBox lib
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceBox
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)
LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/slice --include-dir IceBox --dll-export ICE_BOX_API
LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(LOCAL_SRCDIR)/Exception.cpp
LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil
include $(LIBRARY_RULES)

#
# IceBox server
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = icebox
ifeq ($(CPP11),yes)
    LOCAL_EXE                   = icebox++11
endif
LOCAL_PATH                      = cpp/src/IceBox
LOCAL_DEPENDENT_DYMODULES       = IceBox Ice IceUtil
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(LOCAL_SRCDIR)/Service.cpp $(LOCAL_SRCDIR)/ServiceManagerI.o
LOCAL_LDFLAGS                   = -rdynamic
include $(APPLICATION_RULES)

#
# IceBox admin
#

ifneq ($(CPP11),yes)

include $(CLEAR_RULES)
LOCAL_MODULE                    = iceboxadmin
LOCAL_PATH                      = cpp/src/IceBox
LOCAL_DEPENDENT_DYMODULES       = IceBox Ice IceUtil
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(LOCAL_SRCDIR)/Admin.o
LOCAL_LDFLAGS                   = -rdynamic
include $(APPLICATION_RULES)

endif