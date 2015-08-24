#
# IcePy
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IcePy
LOCAL_PATH                      = python


LOCAL_SRCDIR                    = ice/python/modules/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_CPPFLAGS                  = -I$(LOCAL_SRCDIR) -Icpp/src/Ice -I$(python_include_dir)
LOCAL_DEPENDENT_MODULES         = mcpp
LOCAL_DEPENDENT_DYMODULES       = IceSSL Ice Slice IceUtil

LOCAL_LIBDIR                    = python
LOCAL_DYLIBNAME                 = $(LOCAL_MODULE).so
LOCAL_DYLIBSONAME               = $(LOCAL_MODULE).so.$(SOVERSION)
LOCAL_DYLIBFILENAME             = $(LOCAL_MODULE).so.$(VERSION)

include $(DYNAMICLIBRARY_RULES)

#
# Python Ice module
#
include $(CLEAR_RULES)

LOCAL_PATH                      = python
LOCAL_PREFIX                    = Ice_
LOCAL_MODULE                    = $(LOCAL_PREFIX)_python
LOCAL_SLICES                    = $(filter-out Metrics.ice, $(wildcard ice/slice/Ice/*.ice))
LOCAL_SLICE2PYFLAGS             = --ice --checksum --prefix $(LOCAL_PREFIX) --no-package

include $(PYTHONPACKAGE_RULES)

#
# Python Metrics module
#
include $(CLEAR_RULES)

LOCAL_PREFIX                    = Ice_
LOCAL_MODULE                    = IceMX_python
LOCAL_SLICES                    = ice/slice/Ice/Metrics.ice
LOCAL_SLICE2PYFLAGS             = --ice --checksum --prefix --no-package $(LOCAL_PREFIX)

include $(PYTHONPACKAGE_RULES)

#
# Python Glacier2 module
#
include $(CLEAR_RULES)

LOCAL_PREFIX                    = 
LOCAL_MODULE                    = Glacier2_python
LOCAL_SLICES                    = $(wildcard ice/slice/Glacier2/*.ice)
LOCAL_SLICE2PYFLAGS             = --ice --checksum --prefix Glacier2_ --no-package

include $(PYTHONPACKAGE_RULES)

#
# Python IceBox module
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceBox_python
LOCAL_SLICES                    = $(wildcard ice/slice/IceBox/*.ice)
LOCAL_SLICE2PYFLAGS             = --ice --checksum --prefix IceBox_

include $(PYTHONPACKAGE_RULES)

#
# Python IceGrid module
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceGrid_python
LOCAL_SLICES                    = $(wildcard ice/slice/IceGrid/*.ice)
LOCAL_SLICE2PYFLAGS             = --ice --checksum --prefix IceGrid_ --no-package

include $(PYTHONPACKAGE_RULES)

#
# Python IceStorm module
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IceStorm_python
LOCAL_SLICES                    = $(wildcard ice/slice/IceStorm/*.ice)
LOCAL_SLICE2PYFLAGS             = --ice --checksum --prefix IceStorm_ --no-package

include $(PYTHONPACKAGE_RULES)
