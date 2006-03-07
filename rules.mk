# below are generic rules.

COMMA=,

# various OS-specific commands
CC=gcc
CXX=g++
RM=rm -f
RM_RECURSE_DIR=rm -rf
CP=cp
TAR=tar -zcf
UNTAR=tar -zxf
MKDIR=XXX

# my personal library of useful make functions.
# these really should be part of the core GNU make
# *sigh*
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
gen-bin=$(strip $(addprefix $(TOP_BUILD_DIR)/,$1))
gen-dep=$(strip $(call gen-bin, \
	$(addsuffix .P,$(basename $(filter %.c,$1))) \
	$(addsuffix .P,$(basename $(filter %.cc,$1))) \
))
gen-obj= $(strip $(addprefix $2, \
	$(addsuffix .o,$(basename $(filter %.c,$1))) \
	$(addsuffix .o,$(basename $(filter %.s,$1))) \
	$(addsuffix .o,$(basename $(filter %.cc,$1))) \
	$(filter %.py,$1) \
))
gen-dirs=$(strip $(sort $(call map,enumerate-sub-dirs,$(dir $1))))
display-compile=$(if $(VERBOSE),echo '$(1)' && $(1),echo 'Building $$@ ...' && $(1))


define gen-gcc-dep
-Wp$(COMMA)-M$(COMMA)-MP$(COMMA)-MM$(COMMA)-MT$(COMMA)$(strip $(2))$(COMMA)-MF$(COMMA)$(call gen-dep, $(1))
endef
define DIR_template
$(1): $$(call enumerate-dep-dirs,$(1))
	@if ! test -d $$@; then echo "mkdir $$@"; mkdir $$@; fi
endef
define CXXOBJ_template
$(2): $(1)
	@$(call display-compile,$(CXX) $(3) $$(call gen-gcc-dep,$(1),$(2)) -c -o $(2) $(1))
endef
define COBJ_template
$(2): $(1)
	@$(call display-compile,$(CC) $(3) $$(call gen-gcc-dep,$(1),$(2)) -c -o $(2) $(1))
endef
define PYOBJ_template
$(2): $(1)
	@$(call display-compile,$(CP) $(1) $(2))
endef
define ASOBJ_template
$(2): $(1)
	@$(call display-compile,$(AS) $(3) -o $(2) $(1))
endef

define OUTPUT_template
ALL_OUTPUT += $(TOP_BUILD_DIR)/$$($(1)_OUTPUT)
$(1)_OBJ = $$(call gen-obj,$$($(1)_SRC),$(TOP_BUILD_DIR)/)
ALL_OBJ += $$($(1)_OBJ)
ALL_SRC += $$($(1)_SRC)
ALL_DEP += $$(call gen-dep,$$($(1)_SRC))
ALL_DIST += $$($(1)_SRC)
ALL_DIST += $$($(1)_HDR)
ALL_DIST += $$($(1)_DIST)
CSRC=$$(filter %.c, $$($(1)_SRC))
CXXSRC=$$(filter %.cc, $$($(1)_SRC))
ASSRC=$$(filter %.s, $$($(1)_SRC))
PYSRC=$$(filter %.py, $$($(1)_SRC))
$$(foreach src,$$(CXXSRC),$$(eval $$(call CXXOBJ_template,$$(src),$$(call gen-obj, $$(src), $(TOP_BUILD_DIR)/),$$($(1)_CXXFLAGS))))
$$(foreach src,$$(CSRC),$$(eval $$(call COBJ_template,$$(src),$$(call gen-obj, $$(src), $(TOP_BUILD_DIR)/),$$($(1)_CFLAGS))))
$$(foreach src,$$(PYSRC),$$(eval $$(call PYOBJ_template,$$(src),$$(call gen-obj, $$(src), $(TOP_BUILD_DIR)/))))
$$(foreach src,$$(ASSRC),$$(eval $$(call ASOBJ_template,$$(src),$$(call gen-obj, $$(src), $(TOP_BUILD_DIR)/),$$($(1)_ASFLAGS))))

$(1)_DIRS=$$(call gen-dirs,$$($(1)_OBJ))
ALL_DIRS+=$$($(1)_DIRS)
$(TOP_BUILD_DIR)/$$($(1)_OUTPUT): $$($(1)_OBJ) 
	@$(call display-compile,$$(CXX) $$($(1)_LDFLAGS) -o $$@ $$(filter %.o,$$^))

endef

$(foreach output,$(ALL),$(eval $(call OUTPUT_template,$(output))))
$(foreach dir,$(sort $(ALL_DIRS)),$(eval $(call DIR_template,$(dir))))

build: $(ALL_DIRS) $(ALL_OUTPUT)

opti:
	$(MAKE) TOP_BUILD_DIR=$(TOP_BUILD_DIR)/opti OPTI_FLAGS="-O3 -DNDEBUG=1"
gcov:
	LDFLAGS=-fprofile-arcs CFLAGS="-fprofile-arcs -ftest-coverage" CXXFLAGS="-fprofile-arcs -ftest-coverage" \
 $(MAKE) TOP_BUILD_DIR=$(TOP_BUILD_DIR)/gcov

opti-arc: build-opti-arc-profile run-opti-arc-profile-hook rebuild-opti-arc-profile
rebuild-opti-arc-profile: 
	$(MAKE) TOP_BUILD_DIR=$(TOP_BUILD_DIR)/opti-arc cleano
	CFLAGS=-fprofile-use CXXFLAGS=-fprofile-use \
 $(MAKE) TOP_BUILD_DIR=$(TOP_BUILD_DIR)/opti-arc OPTI_FLAGS="-O3 -DNDEBUG=1"
build-opti-arc-profile:
	LDFLAGS=-fprofile-generate CFLAGS=-fprofile-generate CXXFLAGS=-fprofile-generate \
 $(MAKE) TOP_BUILD_DIR=$(TOP_BUILD_DIR)/opti-arc OPTI_FLAGS="-O3 -DNDEBUG=1"

# dist/distcheck support
ALL_DIST += $(PACKAGE_DIST)
DIST_OUTPUT=$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
DIST_DIR=$(PACKAGE_NAME)-$(PACKAGE_VERSION)
ALL_DIST_TARGETS=$(addprefix $(DIST_DIR)/,$(ALL_DIST))
ALL_DIST_DIRS=$(call gen-dirs, $(ALL_DIST_TARGETS))
$(DIST_OUTPUT): $(ALL_DIST_DIRS) $(ALL_DIST_TARGETS)
	@echo "Building $@ ..."
	@$(TAR) $@ $(DIST_DIR)
$(ALL_DIST_TARGETS): $(DIST_DIR)/%:%
	@$(CP) $< $@
$(ALL_DIST_DIRS):
	@mkdir -p $@
predist:
	$(RM_RECURSE_DIR) -rf $(DIST_DIR)
dist: predist $(DIST_OUTPUT)
	$(RM_RECURSE_DIR) -rf $(DIST_DIR)
distcheck: dist
	$(UNTAR) zxf $(DIST_OUTPUT)
	$(MAKE) -C $(DIST_DIR)
	$(RM_RECURSE_DIR) -rf $(DIST_DIR)
fastdist: $(DIST_OUTPUT)
fastdistcheck: fastdist
	$(MAKE) -C $(DIST_DIR)
	$(RM_RECURSE_DIR) -rf $(DIST_DIR)

-include $(ALL_DEP)

cleano:
	$(RM) -f $(ALL_OBJ)
clean:
	find ./ -name '*~'|xargs rm -f 2>/dev/null;
	$(RM_RECURSE_DIR) -rf $(TOP_BUILD_DIR) 2>/dev/null;
