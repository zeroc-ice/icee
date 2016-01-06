#
# TestCommon
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = TestCommon
LOCAL_PATH                      = cpp/test/Common

LOCAL_SRCDIR                    = ice/cpp/test/Common
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_SLICEDIR                  = ice/cpp/test/Common
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)

LOCAL_CPPFLAGS                  = -Iice/cpp/test/include -I$(LOCAL_PATH) -I$(LOCAL_SRCDIR) -I$(LOCAL_HEADERPATH)
LOCAL_SLICE2CPPFLAGS            = -I$(LOCAL_SLICEDIR) --dll-export TEST_API

LOCAL_HEADERPATH                = cpp/test/include

include $(LIBRARY_RULES)

test_compile: TestCommon
