#
# slice2py
#

include $(CLEAR_RULES)
LOCAL_MODULE                    = slice2py
LOCAL_PATH                      = cpp/src/slice2py
LOCAL_DEPENDENT_MODULES         = Slice Ice mcpp
LOCAL_SRCDIR                    = ice/$(LOCAL_PATH)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)
include $(APPLICATION_RULES)
