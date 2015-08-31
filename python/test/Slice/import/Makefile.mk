#
# Python Ice module
#
include $(CLEAR_RULES)
LOCAL_PATH                      = python/test/Slice/import
LOCAL_MODULE                    = $(subst /,_,$(LOCAL_PATH))
LOCAL_PACKAGE                   = Test
LOCAL_SLICEDIR                  = ice/$(LOCAL_PATH)
LOCAL_SLICE2PYFLAGS             = --no-package
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
include $(PYTHON_RULES)

test_compile: $(LOCAL_MODULE)