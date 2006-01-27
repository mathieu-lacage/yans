# below are generic rules.

define gen-gcc-dep
-Wp$(COMMA)-M$(COMMA)-MP$(COMMA)-MM$(COMMA)-MT$(COMMA)$(strip $(2))$(COMMA)-MF$(COMMA)$(call gen-dep, $(1))
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
	@$(call display-compile,cp $(1) $(2))
endef
define ASOBJ_template
$(2): $(1)
	@$(call display-compile,$(AS) $(3) -o $(2) $(1))
endef

define OUTPUT_template
ALL_OUTPUT += $$($(1)_OUTPUT)
$(1)_OBJ = $$(call gen-obj, $$($(1)_SRC))
ALL_OBJ += $$($(1)_OBJ)
ALL_SRC += $$($(1)_SRC)
ALL_DEP += $$(call gen-dep, $$($(1)_SRC))
CSRC=$$(filter %.c, $$($(1)_SRC))
CXXSRC=$$(filter %.cc, $$($(1)_SRC))
ASSRC=$$(filter %.s, $$($(1)_SRC))
PYSRC=$$(filter %.py, $$($(1)_SRC))
$$(foreach src,$$(CXXSRC),                         \
	$$(eval $$(call CXXOBJ_template,           \
			$$(src),                   \
			$$(call gen-obj, $$(src)), \
			$$($(1)_CXXFLAGS)          \
		)                                  \
	)                                          \
) 
$$(foreach src,$$(CSRC),                           \
	$$(eval $$(call COBJ_template,             \
			$$(src),                   \
			$$(call gen-obj, $$(src)), \
			$$($(1)_CFLAGS)            \
		)                                  \
	)                                          \
)
$$(foreach src,$$(PYSRC),                          \
	$$(eval $$(call PYOBJ_template,            \
			$$(src),                   \
			$$(call gen-obj, $$(src)), \
		)                                  \
	)                                          \
)
$$(foreach src,$$(ASSRC),                          \
	$$(eval $$(call ASOBJ_template,            \
			$$(src),                   \
			$$(call gen-obj, $$(src)), \
			$$($(1)_ASFLAGS)           \
		)                                  \
	)                                          \
)
$$($(1)_OUTPUT): $$($(1)_OBJ)
	@$(call display-compile,$$(CXX) $$($(1)_LDFLAGS) -o $$@ $$(filter %.o,$$^))
endef

$(foreach output,$(ALL),$(eval $(call OUTPUT_template,$(output))))

build: all_dirs $(ALL_OUTPUT)

ALL_DIRS=$(sort $(call gen-dirs, $(ALL_SRC)))
all_dirs: $(ALL_DIRS)
$(ALL_DIRS):
	mkdir -p $@

-include $(ALL_DEP)

clean:
	find ./ -name '*~'|xargs rm -f 2>/dev/null;
	rm -rf $(TOP_BUILD_DIR) 2>/dev/null;