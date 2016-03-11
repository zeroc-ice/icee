#
# IceStorm
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceStorm
LOCAL_PATH                      = cpp/src/$(LOCAL_MODULE)

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_MODULE)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/slice --include-dir IceStorm --dll-export ICE_STORM_API

LOCAL_SRCDIR                    = ice/cpp/src/$(LOCAL_MODULE)Lib
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
LOCAL_DEPENDENT_DYMODULES       = Ice IceUtil

LOCAL_HEADERPATH                = cpp/include/$(LOCAL_MODULE)

include $(LIBRARY_RULES)

#
# IceStormLocalSlice
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = IceStormServiceLocalSlice
LOCAL_PATH                      = cpp/src/IceStorm

LOCAL_SLICEDIR                  = ice/$(LOCAL_PATH)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/cpp/src --include-dir IceStorm

LOCAL_OBJPREFIX                 = $(OBJPREFIX)/pic
LOCAL_CPPFLAGS                  = -fPIC
LOCAL_SLICEOUTPUTDIR	        = cpp/src/IceStorm

include $(OBJECT_RULES)
ICESTORM_LOCAl_GENOBJS          := $(LOCAL_OBJS)

#
# IceStormService
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = IceStormService
LOCAL_PATH                      = cpp/src/IceStorm

LOCAL_SLICEDIR                  = ice/slice/$(LOCAL_PATH)
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
LOCAL_SLICE2CPPFLAGS            = --ice -Iice/slice -include-dir IceStorm

LOCAL_DEPENDENT_DYMODULES       = Freeze IceStorm IceGrid Glacier2 IceBox Ice IceUtil
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)

LOCAL_FREEZEOUTPUTDIR           = $(LOCAL_PATH)
LOCAL_SLICE2FREEZEFLAGS         = --ice -Iice/cpp/src --include-dir IceStorm

LOCAL_FREEZE_CLASS              = LLUMap
LOCAL_FREEZE_DICT               = IceStorm::LLUMap,string,IceStormElection::LogUpdate LLUMap $(LOCAL_SRCDIR)/LLURecord.ice
LOCAL_FREEZE_DEPS               = $(LOCAL_SRCDIR)/LLURecord.ice
include $(FREEZE_RULES)

LOCAL_FREEZE_CLASS              = SubscriberMap
LOCAL_FREEZE_DICT               = IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
                                  SubscriberMap $(LOCAL_SRCDIR)/SubscriberRecord.ice
LOCAL_FREEZE_DEPS               = $(LOCAL_SRCDIR)/SubscriberRecord.ice ice/slice/Ice/Identity.ice
include $(FREEZE_RULES)


LOCAL_FREEZE_CLASS              = V32FormatDB
LOCAL_FREEZE_DICT               = IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
                                  V32FormatDB $(LOCAL_SRCDIR)/V32Format.ice
LOCAL_FREEZE_DEPS               = $(LOCAL_SRCDIR)/V32Format.ice
include $(FREEZE_RULES)

LOCAL_FREEZE_CLASS              = V31FormatDB
LOCAL_FREEZE_DICT               =  IceStorm::V31Format,string,IceStorm::LinkRecordDict \
                                   V31FormatDB $(LOCAL_SRCDIR)/V31Format.ice
LOCAL_FREEZE_DEPS               = $(LOCAL_SRCDIR)/V31Format.ice
include $(FREEZE_RULES)

LOCAL_SRCS                      = $(filter-out $(LOCAL_SRCDIR)/Admin.cpp \
                                               $(LOCAL_SRCDIR)/Grammar.cpp \
                                               $(LOCAL_SRCDIR)/Parser.cpp \
                                               $(LOCAL_SRCDIR)/Scanner.cpp \
                                               $(LOCAL_SRCDIR)/Migrate.cpp, \
                                               $(wildcard $(LOCAL_SRCDIR)/*.cpp))

LOCAL_OBJS                      = $(ICESTORM_LOCAl_GENOBJS)
LOCAL_LINKWITH                  = -Wl,-Bdynamic -lssl
LOCAL_LDFLAGS                   = -rdynamic
include $(DYNAMICLIBRARY_RULES)

#
# icestormadmin
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = icestormadmin
LOCAL_PATH                      = cpp/src/IceStorm

# LOCAL_SLICEDIR                  = ice/$(LOCAL_PATH)
# LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
# LOCAL_SLICE2CPPFLAGS            = --ice -Icpp/src --include-dir IceStorm

LOCAL_DEPENDENT_DYMODULES       = IceStorm Ice IceUtil
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(LOCAL_SRCDIR)/Admin.cpp \
                                  $(LOCAL_SRCDIR)/Grammar.cpp \
                                  $(LOCAL_SRCDIR)/Parser.cpp \
                                  $(LOCAL_SRCDIR)/Scanner.cpp
LOCAL_OBJS                      = $(ICESTORM_LOCAl_GENOBJS)
LOCAL_LINKWITH                  = -Wl,-Bdynamic -lssl
LOCAL_LDFLAGS                   = -rdynamic
include $(APPLICATION_RULES)
