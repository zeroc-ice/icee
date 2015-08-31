#
# mcpp lib
#
include $(CLEAR_RULES)
LOCAL_MODULE                    = mcpp
LOCAL_SRCDIR                    = mcpp
LOCAL_CFLAGS                    = -fPIC
LOCAL_PATH                      = cpp/src/mcpp
LOCAL_C_SRCS                    = $(wildcard $(LOCAL_SRCDIR)/*.c)
include $(STATICLIBRARY_RULES)