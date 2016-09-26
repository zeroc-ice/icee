#
# Python Ice module
#
ifneq ($(CPP11),yes)
include $(CLEAR_RULES)
LOCAL_PATH                      = python/test/Slice/unicodePaths
LOCAL_SLICES                    = 
include $(PYTHON_RULES)

test_compile: $(LOCAL_MODULE)
endif
