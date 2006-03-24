all:

# a small set of functions needed below to bootstrap 
# the platform guesses.
remove-trailing-slash=$(patsubst %/,%,$(1))
append-file=$(addsuffix /$(1),$(call remove-trailing-slash,$(2)))
find-file=$(wildcard $(call append-file,$(1),$(2)))
path=$(subst :, ,$(PATH))
find-program=$(call find-file,$(1),$(path) $(2))

# various OS/system-specific commands
CC:=gcc
CXX:=g++
RM:=rm
CP:=cp
TAR:=tar -zcf
UNTAR:=tar -zxf
MKDIR:=mkdir
RMDIR:=rmdir
ECHO:=echo

CONF_UNAME_BIN=$(call find-program,uname)
CONF_UNAME_HARD=$(shell $(CONF_UNAME_BIN) -p)
CONF_UNAME_OS=$(shell $(CONF_UNAME_BIN) -s)
CONF_I386=$(if $(findstring i386,$(CONF_UNAME_HARD)),i386,$(if $(findstring i686,$(CONF_UNAME_HARD)),i386))
CONF_PPC=$(if $(findstring powerpc,$(CONF_UNAME_HARD)),ppc,)
CONF_UNAME_LINUX=$(findstring Linux,$(CONF_UNAME_OS))
CONF_UNAME_DARWIN=$(findstring Darwin,$(CONF_UNAME_OS))
CONF_PLATFORM_LINUX=$(if $(CONF_UNAME_LINUX),$(if $(CONF_I386),i386-linux-gcc),)
CONF_PLATFORM_DARWIN=$(if $(CONF_UNAME_DARWIN),$(if $(CONF_PPC),ppc-darwin-gcc),)
guess-platform=$(strip \
$(if $(CONF_PLATFORM_LINUX),$(CONF_PLATFORM_LINUX),\
$(if $(CONF_PLATFORM_DARWIN),$(CONF_PLATFORM_DARWIN),$(warning could not guess platform)\
)))

LOCAL_PLATFORM:=$(strip \
$(if $(PLATFORM),$(PLATFORM),\
$(if $(call guess-platform),$(call guess-platform),$(warning could not detect platform)\
)))
ifeq ($(LOCAL_PLATFORM),i386-linux-gcc)
platform-sharedlib-name=$(addprefix lib,$(addsuffix .so,$(1)))
platform-sharedlib-build-flags=-fPIC
platform-sharedlib-link-flags=-shared
platform-pymod-name=$(addsuffix module.so,$(1))
platform-pymod-build-flags=-I$(PYTHON_PREFIX_INC) -I$(BOOST_PREFIX_INC)
platform-pymod-link-flags=-L$(PYTHON_PREFIX_LIB) -L$(BOOST_PREFIX_LIB) -lboost_python -shared
endif

ifeq ($(LOCAL_PLATFORM),ppc-darwin-gcc)
platform-sharedlib-name=$(addprefix lib,$(addsuffix .dylib,$(1)))
platform-sharedlib-build-flags=-fno-common
platform-sharedlib-link-flags=-dynamiclib
platform-pymod-name=$(addsuffix module.so,$(1))
platform-pymod-build-flags=-I$(PYTHON_PREFIX_INC) -I$(BOOST_PREFIX_INC) -fno-common
platform-pymod-link-flags=-w -bundle -bundle_loader $(PYTHON_BIN) -framework Python -L$(BOOST_PREFIX_LIB) -lboost_python
endif

# my personal library of useful make functions.
# these really should be part of the core GNU make
# *sigh*
comma:=,
find-file-prefix=$(call remove-trailing-slash,$(patsubst %$(1),%,$(wildcard $(call append-file,$(1),$(2)))))
rwildcard=$(foreach dir,$(wildcard $(1)*),$(call rwildcard,$(dir)/)$(filter $(subst *,%,$(2)),$(dir))) 
ld-library-path=$(subst :, ,$(LD_LIBRARY_PATH))
find-library=$(call find-file,$(call platform-sharedlib-name,$(1)),$(2))
find-one-library=$(firstword $(foreach lib,$(1),$(call find-library,$(lib),$(2))))
map=$(foreach tmp,$(2),$(call $(1),$(tmp)))
remove-first=$(wordlist 2,$(words $1),$1)
remove-last=$(call reverse,$(call remove-first,$(call reverse,$1)))
reverse=$(if $1,$(call reverse,$(call remove-first,$1)) $(word 1,$1),)
split-dirs=$(subst /, ,$1)
unsplit-dirs=$(call join-items,$(strip $1),/)
join-items=$(if $1,$(word 1,$1)$2$(call join-items,$(call remove-first,$1),$2),)
sub-lists=$(if $1,$(call sub-lists,$(call remove-first,$1)) $1,)
remove-last-dir=$(call unsplit-dirs,$(call remove-last,$(call split-dirs,$1)))
enumerate-sub-dirs=$(if $1,$(call enumerate-sub-dirs,$(call remove-last-dir,$1)) $1,)
enumerate-dep-dirs=$(if $1,$(call enumerate-dep-dirs,$(call remove-last-dir,$1)) $(call remove-last-dir,$1),)
is-dir=$(findstring $(call remove-trailing-slash,$(1))/,$(wildcard $(call remove-trailing-slash,$(1))/))
is-file=$(if $(call is-dir,$(1)),,$(wildcard $(1)))
is-dirs=$(foreach item,$(1),$(call is-dir,$(item)))
is-files=$(foreach item,$(1),$(call is-file,$(item)))
mkdir-p=$(foreach dir,$(call enumerate-dep-dirs,$(1)) $(1),$(if $(wildcard $(dir)),,$(MKDIR) $(dir);))
rmdir=$(foreach dir,$(call reverse $(call enumerate-dep-dirs,$(1))) $(1),$(if $(call is-dir (dir)),,$(RMDIR) $(dir);))
rm-f=$(foreach file,$(1),$(if $(call is-file,$(1)),$(RM) $(file);,))
rm-rf=$(call rm-f,$(call is-files,$(call rwildcard,$(1),*)))$(call rmdir,$(call is-dirs,$(call rwildcard,$(1),*)))

test-functions:
	#$(call mkdir-p,bin/samples)


#$(call find-header,math.h,/usr/include /include)
#$(call find-program,main-simple,/home/mathieu/code/yans-current/bin/samples)
#$(call find-program,main-simple)
#$(call find-program,uname)
#$(call mkdir-p,a/b/c)
#'$(call is-dir,a)'
#$(call rm-rf,bin)
#$(call is-files,./bin bin/libyans.so)
#$(call rm-f,bin/libyans.so bin/libyans.so.P)

