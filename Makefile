all: build

include ./functions.mk
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
FLAGS=-Wall -Werror -O3 -gdwarf-2 -g3
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
	src/common/callback-test.cc \
	src/common/traced-variable-test.cc \
	src/common/packet-logger.cc \
	src/common/trace-container.cc \
	src/common/pcap-writer.cc \
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
	src/host/loopback-interface.cc \
	src/ethernet/cable.cc \
	src/ethernet/chunk-mac-crc.cc \
	src/ethernet/chunk-mac-llc-snap.cc \
	src/ethernet/ethernet-network-interface.cc \
	src/apps/udp-source.cc \
	src/apps/udp-sink.cc \
	src/apps/tcp-source.cc \
	src/apps/tcp-sink.cc \
	src/apps/periodic-generator.cc \
	src/apps/traffic-analyser.cc \
	test/test.cc \
	$(NULL)
YANS_CXXFLAGS=$(CXXFLAGS) $(call gen-lib-build-flags)
YANS_CFLAGS=$(CFLAGS) $(call gen-lib-build-flags)
YANS_LDFLAGS=$(LDFLAGS) $(call gen-lib-link-flags)
YANS_OUTPUT=$(TOP_BUILD_DIR)/$(call gen-lib-name, yans)
ifeq ($(TCP),bsd)
YANS_SRC += \
	src/ipv4/tcp-bsd/tcp-bsd-connection.cc
YANS_CXXFLAGS += -DTCP_USE_BSD
YANS_CXXFLAGS += -I$(TOP_SRC_DIR)/src/ipv4/tcp-bsd/
endif


# building of main-test
TEST_SRC=test/main-test.cc
TEST_OUTPUT=$(call gen-bin, test/main-test)
TEST_CXXFLAGS=$(CXXFLAGS)
TEST_LDFLAGS=$(LDFLAGS) -lyans -L$(TOP_BUILD_DIR)

# building of sample applications
SAMPLE_CXX_SIMU_SRC= \
	samples/main-forwarding-simulator.cc \
	$(NULL)
SAMPLE_CXX_SIMU_OUTPUT=$(call gen-bin, $(basename $(SAMPLE_CXX_SIMU_SRC)))
SAMPLE_CXX_SIMU_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMU_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans

SAMPLE_CXX_SIMUTEMP_SRC= \
	samples/main-forwarding-simulator-template.cc \
	$(NULL)
SAMPLE_CXX_SIMUTEMP_OUTPUT=$(call gen-bin, $(basename $(SAMPLE_CXX_SIMUTEMP_SRC)))
SAMPLE_CXX_SIMUTEMP_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMUTEMP_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans

SAMPLE_CXX_SIMPLE_SRC= \
	samples/main-simple.cc \
	$(NULL)
SAMPLE_CXX_SIMPLE_OUTPUT=$(call gen-bin, $(basename $(SAMPLE_CXX_SIMPLE_SRC)))
SAMPLE_CXX_SIMPLE_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMPLE_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans

SAMPLE_CXX_ROUTER_SRC= \
	samples/main-router.cc \
	$(NULL)
SAMPLE_CXX_ROUTER_OUTPUT=$(call gen-bin, $(basename $(SAMPLE_CXX_ROUTER_SRC)))
SAMPLE_CXX_ROUTER_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_ROUTER_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans

SAMPLE_CXX_TCP_SRC= \
	samples/main-tcp.cc \
	$(NULL)
SAMPLE_CXX_TCP_OUTPUT=$(call gen-bin, $(basename $(SAMPLE_CXX_TCP_SRC)))
SAMPLE_CXX_TCP_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_TCP_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans

SAMPLE_CXX_THREAD_SRC= \
	samples/main-thread.cc \
	$(NULL)
SAMPLE_CXX_TREAD_OUTPUT=$(call gen-bin, $(basename $(SAMPLE_CXX_THREAD_SRC)))
SAMPLE_CXX_TREAD_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_TREAD_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans




# building of python bindings.
YANS_PYTHON_SRC= \
	python/yans-simulator.cc \
	python/export-simulator.cc \
	python/function-holder.cc \
	python/yans/__init__.py \
	python/yans/simulator/__init__.py \
	python/export-thread.cc \
	python/export-callback.cc \
	python/export-periodic-generator.cc \
	python/export-traffic-analyser.cc \
	python/export-packet.cc \
	python/export-ipv4-address.cc \
	python/export-mac-address.cc \
	python/export-host.cc \
	python/export-udp-sink.cc \
	python/export-udp-source.cc \
	python/export-network-interface.cc \
	python/export-ethernet-network-interface.cc \
	python/export-cable.cc \
	python/export-ipv4-route.cc \
	python/export-trace-container.cc \
	python/export-pcap-writer.cc \
	python/test-periodic-generator.py \
	samples/test-simulator.py \
	samples/test-thread.py \
	samples/test-simulator-gc.py \
	samples/test-simple.py \
	$(NULL)
YANS_PYTHON_OUTPUT=$(TOP_BUILD_DIR)/python/$(call gen-pymod-name, _yans)
YANS_PYTHON_CXXFLAGS=$(CXXFLAGS) $(call gen-pymod-build-flags)
YANS_PYTHON_LDFLAGS=$(LDFLAGS) $(call gen-pymod-link-flags) -lyans -L$(TOP_BUILD_DIR)


ALL= \
	YANS \
	TEST \
	SAMPLE_CXX_SIMPLE \
	SAMPLE_CXX_ROUTER \
	SAMPLE_CXX_TCP \
	SAMPLE_CXX_SIMU \
	SAMPLE_CXX_SIMUTEMP \
	SAMPLE_CXX_THREAD \
	YANS_PYTHON \
	$(NULL)

include rules.mk
