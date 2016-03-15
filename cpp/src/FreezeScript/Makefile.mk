#
# FreezeScript tools
#

ifneq ($(CPP11),yes)

include $(CLEAR_RULES)
LOCAL_MODULE                    = transformdb
LOCAL_PATH                      = cpp/src/FreezeScript

LOCAL_DEPENDENT_DYMODULES       = IceUtil Slice Ice IceXML Freeze
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(filter-out $(LOCAL_SRCDIR)/DumpDescriptors.cpp \
					       $(LOCAL_SRCDIR)/DumpDB.cpp, \
					       $(wildcard $(LOCAL_SRCDIR)/*.cpp))

LOCAL_LINKWITH                  = -Wl,-Bdynamic -ldb_cxx-5.3
LOCAL_LDFLAGS                   = -rdynamic
include $(APPLICATION_RULES)


include $(CLEAR_RULES)
LOCAL_MODULE                    = dumpdb
LOCAL_PATH                      = cpp/src/FreezeScript

LOCAL_DEPENDENT_DYMODULES       = IceUtil Slice Ice IceXML Freeze
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(filter-out $(LOCAL_SRCDIR)/TransformAnalyzer.cpp \
					       $(LOCAL_SRCDIR)/TransformVisitor.cpp \
					       $(LOCAL_SRCDIR)/Transformer.cpp \
					       $(LOCAL_SRCDIR)/transformdb.cpp, \
					       $(wildcard $(LOCAL_SRCDIR)/*.cpp))

LOCAL_LINKWITH                  = -Wl,-Bdynamic -ldb_cxx-5.3
LOCAL_LDFLAGS                   = -rdynamic
include $(APPLICATION_RULES)

endif