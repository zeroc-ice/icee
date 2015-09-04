#
# IcePy
#
include $(CLEAR_RULES)

LOCAL_MODULE                    = IcePy
LOCAL_PATH                      = python/python$(PYTHON_BASEVERSION)

LOCAL_SRCDIR                    = ice/python/modules/$(LOCAL_MODULE)
LOCAL_SRCS                      = $(wildcard $(LOCAL_SRCDIR)/*.cpp)

LOCAL_CPPFLAGS                  = -I$(LOCAL_SRCDIR) -Icpp/src/Ice -I$(python_include_dir)
LOCAL_DEPENDENT_MODULES         = mcpp
LOCAL_DEPENDENT_DYMODULES       = IceSSL Ice Slice IceUtil

LOCAL_LIBDIR                    = $(LOCAL_PATH)
LOCAL_DYLIBNAME                 = $(LOCAL_MODULE).so
LOCAL_DYLIBSONAME               = $(LOCAL_MODULE).so.$(SOVERSION)
LOCAL_DYLIBFILENAME             = $(LOCAL_MODULE).so.$(VERSION)

include $(DYNAMICLIBRARY_RULES)

METRICS_SLICES                  = ice/slice/Ice/Metrics.ice ice/slice/Glacier2/Metrics.ice ice/slice/IceStorm/Metrics.ice

#
# Python Ice module
#
include $(CLEAR_RULES)
LOCAL_PREFIX                    = Ice_
LOCAL_SLICEDIR                  = ice/slice/Ice
LOCAL_SLICES                    = $(wildcard ice/slice/Ice/*.ice)
include $(ICEE_PYTHON_RULES)

#
# Python Glacier2 module
#
include $(CLEAR_RULES)
LOCAL_PREFIX                    = Glacier2_
LOCAL_SLICEDIR                  = ice/slice/Glacier2
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
include $(ICEE_PYTHON_RULES)

#
# Python IceBox module
#
include $(CLEAR_RULES)
LOCAL_PACKAGE                   = IceBox
LOCAL_PREFIX                    = IceBox_
LOCAL_SLICEDIR                  = ice/slice/IceBox
LOCAL_SLICES                    = $(wildcard $(LOCAL_SLICEDIR)/*.ice)
include $(ICEE_PYTHON_RULES)

#
# Python IceGrid module
#
include $(CLEAR_RULES)
LOCAL_PACKAGE                   = IceGrid
LOCAL_PREFIX                    = IceGrid_
LOCAL_SLICEDIR                  = ice/slice/IceGrid
LOCAL_SLICES                    = $(filter-out $(LOCAL_SLICEDIR)/PluginFacade.ice, $(wildcard $(LOCAL_SLICEDIR)/*.ice))
include $(ICEE_PYTHON_RULES)

#
# Python IceStorm module
#
include $(CLEAR_RULES)
LOCAL_PACKAGE                   = IceStorm
LOCAL_PREFIX                    = IceStorm_
LOCAL_SLICEDIR                  = ice/slice/IceStorm
LOCAL_SLICES                    = $(wildcard ice/slice/IceStorm/*.ice)
include $(ICEE_PYTHON_RULES)

#
# Python Ice and Glacier2
#
IcePy: python/python$(PYTHON_BASEVERSION)/Ice.py python/python$(PYTHON_BASEVERSION)/Glacier2.py

python/python$(PYTHON_BASEVERSION)/Ice.py:
	$(E) "Copying Ice package to python/python$(PYTHON_BASEVERSION)/"
	$(Q)cp ice/python/python/Ice.py python/python$(PYTHON_BASEVERSION)/

python/python$(PYTHON_BASEVERSION)/Glacier2.py:
	$(E) "Copying Glacier2 package to python/python$(PYTHON_BASEVERSION)/"
	$(Q)cp ice/python/python/Glacier2.py python/python$(PYTHON_BASEVERSION)/
#
# Python IceMX module
#
IcePy: python/python$(PYTHON_BASEVERSION)/IceMX/__init__.py

python/python$(PYTHON_BASEVERSION)/IceMX/__init__.py: $(METRICS_SLICES)
	$(E) "Generating IceMX package index"
	$(Q)$(SLICE2PY) --output-dir python/python$(PYTHON_BASEVERSION) --build-package --ice $(SLICE2PYFLAGS) --prefix Ice_ ice/slice/Ice/Metrics.ice
	$(Q)$(SLICE2PY) --output-dir python/python$(PYTHON_BASEVERSION) --build-package --ice $(SLICE2PYFLAGS) --prefix Glacier2_ ice/slice/Glacier2/Metrics.ice
	$(Q)$(SLICE2PY) --output-dir python/python$(PYTHON_BASEVERSION) --build-package --ice $(SLICE2PYFLAGS) --prefix IceStorm_ ice/slice/IceStorm/Metrics.ice

IceMX_clean:
	$(Q)rm -rf python/python$(PYTHON_BASEVERSION)/IceMX

CLEAN_TARGETS := $(CLEAN_TARGETS) IceMX_clean