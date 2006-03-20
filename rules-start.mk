# various OS-specific commands
CC:=gcc
CXX:=g++
RM:=rm
CP:=cp
TAR:=tar -zcf
UNTAR:=tar -zxf
MKDIR:=mkdir
RMDIR:=rmdir


# below are generic rules.

comma:=,

# my personal library of useful make functions.
# these really should be part of the core GNU make
# *sigh*
remove-trailing-slash=$(patsubst %/,%,$(1))
append-file=$(addsuffix /$(1),$(call remove-trailing-slash,$(2)))
find-file=$(wildcard $(call append-file,$(1),$(2)))
find-file-prefix=$(call remove-trailing-slash,$(patsubst %$(1),%,$(wildcard $(call append-file,$(1),$(2)))))
rwildcard=$(foreach dir,$(wildcard $(1)*),$(call rwildcard,$(dir)/)$(filter $(subst *,%,$(2)),$(dir))) 
path=$(subst :, ,$(PATH))
ld-library-path=$(subst :, ,$(LD_LIBRARY_PATH))
find-program=$(call find-file,$(1),$(path) $(2))
find-library=$(call find-file,lib$(1).so,$(2))
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


all: build

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

