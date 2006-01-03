TOP=.
NULL=


genbin=$(addprefix $(TOP_INSTALL)/, $1)
genobj= $(call genbin, \
	$(addsuffix .o, $(basename $(filter %.c,$1))) \
	$(addsuffix .o, $(basename $(filter %.cc,$1))) \
	$(filter %.py,$1) \
)
gendirs=$(sort $(dir $(call genobj, $1)))
