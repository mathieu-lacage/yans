gen-bin=$(strip $(addprefix $(TOP_BUILD_DIR)/,$1))
gen-dep=$(strip $(call gen-bin, \
	$(addsuffix .P,$(filter %.c,$1)) \
	$(addsuffix .P,$(filter %.cc,$1)) \
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
# the following templates are used to generate the targets for all object files.
# each .o depends only on its source file. They do not depend on the build dirs
# which could be generated with a call to enumerate-dep-dirs because 
# the dir targets are always re-made which would trigger a re-make for all 
# object files all the time.
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
$$(foreach src,$$(CXXSRC),$$(eval $$(call CXXOBJ_template,$$(src),$$(call gen-obj,$$(src),$(TOP_BUILD_DIR)/),$$($(1)_CXXFLAGS))))
$$(foreach src,$$(CSRC),$$(eval $$(call COBJ_template,$$(src),$$(call gen-obj,$$(src),$(TOP_BUILD_DIR)/),$$($(1)_CFLAGS))))
$$(foreach src,$$(PYSRC),$$(eval $$(call PYOBJ_template,$$(src),$$(call gen-obj,$$(src),$(TOP_BUILD_DIR)/))))
$$(foreach src,$$(ASSRC),$$(eval $$(call ASOBJ_template,$$(src),$$(call gen-obj,$$(src),$(TOP_BUILD_DIR)/),$$($(1)_ASFLAGS))))

$(1)_DIRS=$$(call gen-dirs,$$($(1)_OBJ))
ALL_DIRS+=$$($(1)_DIRS)
# the output.P dependency files are used to work around a gnu make bug:
# gnu make 3.80 cannot deal with long dependency lists in targets located 
# in templates called with a eval. So, instead of making output depend
# directly on the object list, we store the object list in the output.P
# file and make sure output.P is included later by adding to the ALL_DEP
# list. fun fun.
ALL_DEP+=$(TOP_BUILD_DIR)/$$($(1)_OUTPUT).P
$(TOP_BUILD_DIR)/$$($(1)_OUTPUT).P: $$(call enumerate-dep-dirs,$(TOP_BUILD_DIR)/$$($(1)_OUTPUT).P)
	@echo $(TOP_BUILD_DIR)/$$($(1)_OUTPUT): $$($(1)_OBJ) > $$@
$(TOP_BUILD_DIR)/$$($(1)_OUTPUT):
	@$(call display-compile,$$(CXX) $$($(1)_LDFLAGS) -o $$@ $$(filter %.o,$$^))
endef

$(foreach output,$(ALL),$(eval $(call OUTPUT_template,$(output))))
$(sort $(ALL_DIRS)):
	$(call mkdir-p,$@)
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
$(sort $(ALL_DIST_DIRS)):
	$(call mkdir-p,$@)
$(DIST_OUTPUT): $(ALL_DIST_DIRS) $(ALL_DIST_TARGETS)
	@echo "Building $@ ..."
	@$(TAR) $@ $(DIST_DIR)
$(ALL_DIST_TARGETS): $(DIST_DIR)/%:%
	@$(CP) $< $@
predist:
	$(call rm-rf,$(DIST_DIR))
dist: predist $(DIST_OUTPUT)
	$(call rm-rf,$(DIST_DIR))
distcheck: dist
	$(UNTAR) $(DIST_OUTPUT)
	$(MAKE) -C $(DIST_DIR)
	$(call rm-rf,$(DIST_DIR))
fastdist: $(DIST_OUTPUT)
fastdistcheck: fastdist
	$(MAKE) -C $(DIST_DIR)
	$(call rm-rf,$(DIST_DIR))

NINCLUDE_TARGETS := \
	clean \
	$(NULL)

ifeq ($(strip $(filter $(NINCLUDE_TARGETS),$(MAKECMDGOALS))),)
ifneq ($(ALL_DEP),)
-include $(ALL_DEP)
endif
endif

cleano:
	$(call rm-f,$(ALL_OBJ))
clean:
	$(call rm-f,$(call rwildcard,./,*~))
	$(call rm-rf,$(TOP_BUILD_DIR))
