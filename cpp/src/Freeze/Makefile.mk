#
# Freeze
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = FreezeLocalSlices
LOCAL_PATH                      = cpp/src/Freeze

LOCAL_SLICEDIR                  = ice/$(LOCAL_PATH)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/slice --include-dir Freeze

LOCAL_OBJPREFIX                 = $(OBJPREFIX)/pic
LOCAL_CPPFLAGS                  = -fPIC
LOCAL_SLICEOUTPUTDIR	        = cpp/src/Freeze

include $(OBJECT_RULES)
FREEZE_LOCAL_GENOBJS           := $(LOCAL_OBJS)

include $(CLEAR_RULES)

LOCAL_MODULE                    = Freeze
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/slice --include-dir Freeze --dll-export FREEZE_API

LOCAL_FREEZEOUTPUTDIR           = cpp/include/$(LOCAL_MODULE)
LOCAL_SLICE2FREEZEFLAGS         := $(LOCAL_SLICE2CPPFLAGS)
LOCAL_FREEZE_CLASS              = Catalog
LOCAL_FREEZE_DICT               = Freeze::Catalog,string,Freeze::CatalogData Catalog ice/slice/Freeze/CatalogData.ice
LOCAL_FREEZE_DEPS               = $(LOCAL_SLICEDIR)/CatalogData.ice
include $(FREEZE_RULES)

LOCAL_FREEZE_CLASS              = CatalogIndexList
LOCAL_FREEZE_DICT               = Freeze::CatalogIndexList,string,Ice::StringSeq CatalogIndexList ice/slice/Ice/BuiltinSequences.ice
LOCAL_FREEZE_DEPS               = ice/slice/Ice/BuiltinSequences.ice
include $(FREEZE_RULES)

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_OBJS                      = $(FREEZE_LOCAL_GENOBJS)
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil
LOCAL_LINKWITH                  = -Wl,-Bdynamic -ldb_cxx-5.3

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

# $(error $(LOCAL_FREEZE_SRCS))

include $(DYNAMICLIBRARY_RULES)
