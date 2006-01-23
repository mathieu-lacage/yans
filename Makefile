include ./functions.mk

all: dirs build build-python

include ./platform.mk

TOP_BUILD_DIR=$(TOP)/bin
TOP_SRC_DIR=$(TOP)
DEFINES=-DRUN_SELF_TESTS=1
INCLUDES=\
 -I$(TOP_SRC_DIR)/simulator \
 -I$(TOP_SRC_DIR)/src/thread \
 -I$(TOP_SRC_DIR)/test \
 -I$(TOP_SRC_DIR)/src/common \
 -I$(TOP_SRC_DIR)/src/host \
 -I$(TOP_SRC_DIR)/src/arp \
 -I$(TOP_SRC_DIR)/src/ethernet \
 -I$(TOP_SRC_DIR)/src/ipv4 \
 -I$(TOP_SRC_DIR)/src/os-model \
 -I$(TOP_SRC_DIR)/src/apps \
 $(NULL)
FLAGS=-Wall -Werror -O0 -g2
PYTHON_PREFIX_INC=/usr/include/python2.3
PYTHON_PREFIX_LIB=/usr/lib
PYTHON_BIN=/usr/bin/python2.3
BOOST_PREFIX_LIB=/usr/lib
BOOST_PREFIX_INC=/usr/include
#TCP=bsd

LDFLAGS=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)


# building of libyans.so
YANS_SRC= \
	simulator/clock.cc \
	simulator/event-heap.cc \
	simulator/event.cc \
	simulator/simulator.cc \
	$(FIBER_CONTEXT_PLATFORM) \
	src/thread/semaphore.cc \
	src/thread/fiber.cc \
	src/thread/fiber-scheduler.cc \
	src/thread/thread.cc \
	src/common/buffer.cc \
	src/common/chunk.cc \
	src/common/packet.cc \
	src/common/chunk-constant-data.cc \
	src/common/ipv4-address.cc \
	src/common/mac-address.cc \
	src/common/tag-manager.cc \
	src/common/utils.cc \
	src/common/population-analysis.cc \
	src/common/data-writer.cc \
	src/ipv4/tag-ipv4.cc \
	src/ipv4/ipv4-end-point.cc \
	src/ipv4/ipv4-end-points.cc \
	src/ipv4/ipv4-route.cc \
	src/ipv4/ipv4.cc \
	src/ipv4/defrag-state.cc \
	src/ipv4/chunk-ipv4.cc \
	src/ipv4/udp.cc \
	src/ipv4/chunk-udp.cc \
	src/ipv4/chunk-icmp.cc \
	src/ipv4/chunk-tcp.cc \
	src/ipv4/tcp.cc \
	src/ipv4/tcp-connection-listener.cc \
	src/ipv4/tcp-connection.cc \
	src/ipv4/tcp-buffer.cc \
	src/arp/arp.cc \
	src/arp/chunk-arp.cc \
	src/arp/arp-cache-entry.cc \
	src/host/host.cc \
	src/host/network-interface.cc \
	src/host/network-interface-tracer.cc \
	src/host/loopback-interface.cc \
	src/host/host-tracer.cc \
	src/ethernet/cable.cc \
	src/ethernet/chunk-mac-crc.cc \
	src/ethernet/chunk-mac-llc-snap.cc \
	src/ethernet/ethernet-network-interface.cc \
	src/apps/udp-source.cc \
	src/apps/udp-sink.cc \
	src/apps/tcp-source.cc \
	src/apps/tcp-sink.cc \
	src/apps/periodic-generator.cc \
	src/apps/traffic-analyzer.cc \
	test/test.cc \
	$(NULL)
ifeq ($(TCP),bsd)
YANS_SRC += \
	src/ipv4/tcp-bsd/tcp-bsd-connection.cc
CXXFLAGS += -DTCP_USE_BSD
CXXFLAGS += -I$(TOP_SRC_DIR)/src/ipv4/tcp-bsd/
endif
YANS_OBJ=$(call gen-obj, $(YANS_SRC))
LIB_YANS=$(TOP_BUILD_DIR)/$(call gen-lib-name, yans)
$(YANS_OBJ): CXXFLAGS += $(call gen-lib-build-flags)
$(YANS_OBJ): CFLAGS += $(call gen-lib-build-flags)
$(LIB_YANS): LDFLAGS += $(call gen-lib-link-flags)
$(LIB_YANS): $(YANS_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(filter %.o,$^)
DIRS += $(call gen-dirs, $(YANS_SRC))
build: $(LIB_YANS)


# building of main-test
MAIN_TEST_SRC=test/main-test.cc
MAIN_TEST_OBJ=$(call gen-obj, $(MAIN_TEST_SRC))
MAIN_TEST=$(call gen-bin, test/main-test)
$(MAIN_TEST): $(MAIN_TEST_OBJ)
	$(CXX) $(CXXFLAGS) -lyans -L$(TOP_BUILD_DIR) -o $@ $^
DIRS += $(call gen-dirs, $(YANS_SRC))
build: $(MAIN_TEST)


# building of python bindings.
SIMULATOR_PYTHON_SRC= \
	python/yans-simulator.cc \
	python/export-simulator.cc \
	python/export-event.cc \
	python/yans/__init__.py \
	python/yans/simulator/__init__.py \
	python/test-simulator.py \
	python/test-simulator-gc.py \
	$(NULL)
SIMULATOR_PYTHON_OBJ=$(call gen-obj, $(SIMULATOR_PYTHON_SRC))
LIB_SIMULATOR_PYTHON=$(TOP_BUILD_DIR)/python/$(call gen-pymod-name, _simulator)
$(SIMULATOR_PYTHON_OBJ): CXXFLAGS+=$(call gen-pymod-build-flags)
$(LIB_SIMULATOR_PYTHON): LDFLAGS+=$(call gen-pymod-link-flags) -lyans -L$(TOP_BUILD_DIR)
$(LIB_SIMULATOR_PYTHON): $(SIMULATOR_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(filter %.o,$^)
DIRS += $(call gen-dirs, $(SIMULATOR_PYTHON_SRC))
build-python: $(LIB_SIMULATOR_PYTHON)

# building of python models bindings
MODELS_PYTHON_SRC= \
	python/yans-models.cc \
	python/export-thread.cc \
	python/test-thread.py \
	$(NULL)
#	python/export-packet.cc \
#	python/export-ipv4-address.cc \
#	python/export-periodic-generator.cc \
#	python/test-periodic-generator.py \

MODELS_PYTHON_OBJ=$(call gen-obj, $(MODELS_PYTHON_SRC))
LIB_MODELS_PYTHON=$(TOP_BUILD_DIR)/python/$(call gen-pymod-name, _models)
$(MODELS_PYTHON_OBJ): CXXFLAGS+=$(call gen-pymod-build-flags)
$(LIB_MODELS_PYTHON): LDFLAGS+=$(call gen-pymod-link-flags) -lyans -L$(TOP_BUILD_DIR)
$(LIB_MODELS_PYTHON): $(MODELS_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(filter %.o,$^)
DIRS += $(call gen-dirs, $(MODELS_PYTHON_SRC))
build-python: $(LIB_MODELS_PYTHON)

# building of sample applications
SAMPLES_SRC= \
	src/samples/main-simulator.cc \
	src/samples/main-forwarding-simulator.cc \
	src/samples/main-forwarding-simulator-template.cc \
	src/samples/main-simple.cc \
	src/samples/main-router.cc \
	src/samples/main-tcp.cc \
	$(NULL)

DIRS += $(call gen-dirs, $(SAMPLES_SRC))
SAMPLES=$(call gen-bin, $(basename $(SAMPLES_SRC)))
$(SAMPLES): %:%.o
	$(CXX) $(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans -o $@ $^
build: $(SAMPLES)


# below are generic rules.

TMP_DIRS=$(sort $(DIRS))
dirs: $(TMP_DIRS)
$(TMP_DIRS):
	mkdir -p $@;

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@

$(TOP_BUILD_DIR)/%.o:%.s
        $(AS) $(ASFLAGS) -o $@ $<
$(TOP_BUILD_DIR)/%.py:%.py
	cp $< $@
$(TOP_BUILD_DIR)/%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(TOP_BUILD_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: depsclean
	find ./ -name '*~'|xargs rm -f 2>/dev/null;
	rm -rf $(TOP_BUILD_DIR) 2>/dev/null;

depsclean:
	rm -f .deps 2>/dev/null

.deps:
	@echo "Generating dependency information.";
	@rm -f .deps 2>/dev/null; touch .deps;
	@-for f in `find . -name '*.c'`; do \
		TARGET=`echo $(TOP_BUILD_DIR)/$$f|sed -e 's/\.c$$/\.o/' -e 's/\/\.\//\//'`; \
		$(CC) $(CFLAGS) -E -M -MP -MM -MT $$TARGET $$f 2>/dev/null >> .deps; \
	done;
	@-for f in `find . -name '*.cc'`; do \
		TARGET=`echo $(TOP_BUILD_DIR)/$$f|sed -e 's/\.cc$$/\.o/' -e 's/\/\.\//\//'`; \
		$(CXX) $(CXXFLAGS) -E -M -MP -MM -MT $$TARGET $$f 2>/dev/null >> .deps; \
	done;

-include .deps
