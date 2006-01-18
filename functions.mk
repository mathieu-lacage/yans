TOP=.
NULL=


gen-bin=$(addprefix $(TOP_BUILD_DIR)/, $1)
gen-obj= $(call gen-bin, \
	$(addsuffix .o, $(basename $(filter %.c,$1))) \
	$(addsuffix .o, $(basename $(filter %.cc,$1))) \
	$(filter %.py,$1) \
)
gen-dirs=$(sort $(dir $(call gen-obj, $1)))
