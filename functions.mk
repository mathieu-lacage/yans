TOP=.
NULL=
COMMA=,


gen-bin=$(strip $(addprefix $(TOP_BUILD_DIR)/, $1))
gen-dep=$(strip $(call gen-bin, \
	$(addsuffix .P, $(basename $(filter %.c,$1))) \
	$(addsuffix .P, $(basename $(filter %.cc,$1))) \
))
gen-obj= $(strip $(call gen-bin, \
	$(addsuffix .o, $(basename $(filter %.c,$1))) \
	$(addsuffix .o, $(basename $(filter %.s,$1))) \
	$(addsuffix .o, $(basename $(filter %.cc,$1))) \
	$(filter %.py,$1) \
))
gen-dirs=$(strip $(sort $(dir $1)))

display-compile=$(if $(VERBOSE),echo '$(1)' && $(1),echo 'Building $$@ ...' && $(1))
