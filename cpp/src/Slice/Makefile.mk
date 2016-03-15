#
# Slice lib
#
ifneq ($(CPP11),yes)

include $(CLEAR_RULES)

LOCAL_MODULE                    = Slice
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)
LOCAL_DEPENDENT_MODULES         = mcpp
LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_OBJS                      = $(LIBRARY_OBJS)
LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)
LOCAL_DEPENDENT_DYMODULES       = IceUtil
include $(LIBRARY_RULES)

Slice: Slice_dynamiclib

Slice_headers_install: Slice
	$(Q)mkdir -p $(DESTDIR)$(ice_install_include_dir)/Slice
	$(Q)cp cpp/include/Slice/*.h $(DESTDIR)$(ice_install_include_dir)/Slice 

INSTALL_TARGETS := $(INSTALL_TARGETS) Slice_headers_install

endif
