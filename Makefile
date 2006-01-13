include ./functions.mk

TOP_INSTALL=$(TOP)/bin
TOP_PYTHON_INSTALL=$(TOP_INSTALL)/yans
DEFINES=-DRUN_SELF_TESTS=1
INCLUDES=\
 -I$(TOP)/simulator \
 -I$(TOP)/src/thread \
 -I$(TOP)/test \
 -I$(TOP)/src/common \
 -I$(TOP)/src/host \
 -I$(TOP)/src/arp \
 -I$(TOP)/src/ethernet \
 -I$(TOP)/src/ipv4 \
 -I$(TOP)/src/os-model \
 -I$(TOP)/src/apps \
 $(NULL)
FLAGS=-Wall -Werror -O0 -gdwarf-2
PYTHON_INCLUDES=-I/usr/include/python2.3
#TCP=bsd

LDFLAGS=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
all: dirs build


# building of libyans.so
YANS_SRC= \
	simulator/clock.cc \
	simulator/event-heap.cc \
	simulator/event.cc \
	simulator/simulator.cc \
	src/thread/fiber-context-x86-linux-gcc.cc \
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
	src/arp/arp.cc \
	src/arp/chunk-arp.cc \
	src/arp/arp-cache-entry.cc \
	src/host/host.cc \
	src/host/network-interface.cc \
	src/host/network-interface-tracer.cc \
	src/host/loopback-interface.cc \
	src/host/host-tracer.cc \
	src/os-model/write-file.cc \
	src/ethernet/cable.cc \
	src/ethernet/chunk-mac-crc.cc \
	src/ethernet/chunk-mac-llc-snap.cc \
	src/ethernet/ethernet-network-interface.cc \
	src/apps/udp-source.cc \
	src/apps/udp-sink.cc \
	src/apps/periodic-generator.cc \
	src/apps/traffic-analyzer.cc \
	test/test.cc \
	$(NULL)
ifeq ($(TCP),bsd)
YANS_SRC += \
	src/ipv4/tcp-bsd/tcp-bsd-connection.cc
CXXFLAGS += -DTCP_USE_BSD
CXXFLAGS += -I$(TOP)/src/ipv4/tcp-bsd/
endif
YANS_OBJ=$(call genobj, $(YANS_SRC))
LIB_YANS=$(TOP_INSTALL)/libyans.so
$(YANS_OBJ): CXXFLAGS += -fPIC
$(YANS_OBJ): CFLAGS += -fPIC
$(LIB_YANS): $(YANS_OBJ)
	$(CXX) $(LDFLAGS) -shared -o $@ $(filter %.o,$^)
DIRS += $(call gendirs, $(YANS_SRC))
build: $(LIB_YANS)


# building of main-test
MAIN_TEST_SRC=test/main-test.cc
MAIN_TEST_OBJ=$(call genobj, $(MAIN_TEST_SRC))
MAIN_TEST=$(call genbin, test/main-test)
$(MAIN_TEST): $(MAIN_TEST_OBJ)
	$(CXX) $(CXXFLAGS) -lyans -L$(TOP_INSTALL) -o $@ $^
DIRS += $(call gendirs, $(YANS_SRC))
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
SIMULATOR_PYTHON_OBJ=$(call genobj, $(SIMULATOR_PYTHON_SRC))
LIB_SIMULATOR_PYTHON=$(TOP_INSTALL)/python/_simulatormodule.so
$(SIMULATOR_PYTHON_OBJ): CXXFLAGS+=$(PYTHON_INCLUDES)
$(LIB_SIMULATOR_PYTHON): $(SIMULATOR_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP_INSTALL) -lyans -shared -o $@ $(filter %.o,$^)
DIRS += $(call gendirs, $(SIMULATOR_PYTHON_SRC))
build: $(LIB_SIMULATOR_PYTHON)

# building of python models bindings
MODELS_PYTHON_SRC= \
	python/yans-models.cc \
	python/export-thread.cc \
	python/test-thread.py \
	$(NULL)
MODELS_PYTHON_OBJ=$(call genobj, $(MODELS_PYTHON_SRC))
LIB_MODELS_PYTHON=$(TOP_INSTALL)/python/_modelsmodule.so
$(MODELS_PYTHON_OBJ): CXXFLAGS+=$(PYTHON_INCLUDES)
$(LIB_MODELS_PYTHON): $(MODELS_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP_INSTALL) -lyans -shared -o $@ $(filter %.o,$^)
DIRS += $(call gendirs, $(MODELS_PYTHON_SRC))
build: $(LIB_MODELS_PYTHON)

# building of sample applications
SAMPLES_SRC= \
	src/samples/main-simulator.cc \
	src/samples/main-forwarding-simulator.cc \
	src/samples/main-forwarding-simulator-template.cc \
	src/samples/main-simple.cc \
	src/samples/main-router.cc \
	$(NULL)

# 	src/samples/main-router.cc \
# 	src/samples/main-tcp.cc \

DIRS += $(call gendirs, $(SAMPLES_SRC))
SAMPLES=$(call genbin, $(basename $(SAMPLES_SRC)))
$(SAMPLES): %:%.o
	$(CXX) $(LDFLAGS) -L$(TOP_INSTALL) -lyans -o $@ $^
build: $(SAMPLES)



YANS_PYTHON_SRC= \
	$(SIMULATOR_PYTHON_SRC) \
	$(MODELS_PYTHON_SRC) \
	$(NULL)





TMP_DIRS=$(sort $(DIRS))
dirs: $(TMP_DIRS)
$(TMP_DIRS):
	mkdir -p $@;

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@


$(TOP_INSTALL)/%.py:%.py
	cp $< $@
$(TOP_INSTALL)/%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(TOP_INSTALL)/%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: depsclean
	find ./ -name '*~'|xargs rm -f 2>/dev/null;
	rm -rf $(TOP_INSTALL) 2>/dev/null;

depsclean:
	rm -f .deps 2>/dev/null

.deps:
	@echo "Generating dependency information.";
	@rm -f .deps 2>/dev/null; touch .deps;
	@-for f in `find . -name '*.c'`; do \
		TARGET=`echo $(TOP_INSTALL)/$$f|sed -e 's/\.c$$/\.o/' -e 's/\/\.\//\//'`; \
		$(CC) $(CFLAGS) -E -M -MP -MM -MT $$TARGET $$f 2>/dev/null >> .deps; \
	done;
	@-for f in `find . -name '*.cc'`; do \
		TARGET=`echo $(TOP_INSTALL)/$$f|sed -e 's/\.cc$$/\.o/' -e 's/\/\.\//\//'`; \
		$(CXX) $(CXXFLAGS) -E -M -MP -MM -MT $$TARGET $$f 2>/dev/null >> .deps; \
	done;

-include .deps
