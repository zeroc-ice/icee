#
# slice2freeze
#
ifneq ($(CPP11),yes)

include $(CLEAR_RULES)
LOCAL_MODULE                    = slice2freeze
LOCAL_PATH                      = cpp/src/slice2freeze
LOCAL_DEPENDENT_MODULES         = Slice Ice mcpp
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
include $(APPLICATION_RULES)

endif