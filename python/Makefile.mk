#
# IcePy
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IcePy
LOCAL_PATH                      = python

LOCAL_SRCDIR                    = $(LOCAL_PATH)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_CPPFLAGS                  = -I$(LOCAL_SRCDIR) -Icpp/src/Ice -I$(python_include_dir)

LOCAL_LIBDIR                    = $(LOCAL_PATH)
LOCAL_LIBNAME                   = $(LOCAL_MODULE).so
LOCAL_LIBSONAME                 = $(LOCAL_MODULE).so.$(SOVERSION)
LOCAL_FILENAME                  = $(LOCAL_MODULE).so.$(VERSION)


include $(DYNAMICLIBRARY_RULES)

include $(INSTALL_RULES)