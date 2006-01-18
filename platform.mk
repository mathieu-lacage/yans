PLATFORM=i386-linux-gcc

ifeq ($(PLATFORM), i386-linux-gcc)
gen-lib-name=$(addprefix lib, $(addsuffix .so, $1))
gen-lib-build-flags=-fPIC
gen-lib-link-flags=-shared
gen-pymod-name=$(addsuffix module.so, $1)
gen-pymod-build-flags=-I$(PYTHON_PREFIX_INC) -I$(BOOST_PREFIX_INC)
gen-pymod-link-flags=-L$(PYTHON_PREFIX_LIB) -L$(BOOST_PREFIX_LIB) -lboost_python -shared
endif

ifeq ($(PLATFORM), ppc64-darwin-gcc)
gen-lib-name=$(addprefix lib, $(addsuffix .dylib, $1))
gen-lib-build-flags=-fno-common
gen-lib-link-flags=-dynamiclib
gen-pymod-name=$(addsuffix module.so, $1)
gen-pymod-build-flags=-I$(PYTHON_PREFIX_INC) -I$(BOOST_PREFIX_INC) -fno-common
gen-pymod-link-flags=-w -bundle -bundle_loader $(PYTHON_BIN) -L$(PYTHON_PREFIX_LIB) -lpython -L$(BOOST_PREFIX_LIB) -lboost_python
endif
