include ./rules-start.mk
-include ./config.mk
include ./platform.mk

TOP:=.
NULL:=
PACKAGE_NAME:=yans
PACKAGE_VERSION:=0.8.0
TOP_BUILD_DIR:=$(TOP)/bin
TOP_SRC_DIR:=$(TOP)


INCLUDE_SEARCH_PATHS:= \
 /usr/include \
 /include \
 /usr/local/include \
 /usr/include/python2.4 \
 $(NULL)
LIB_SEARCH_PATH:= \
 /usr/lib \
 /lib \
 /usr/local/lib \
 $(NULL)
CONF_B_INC=$(call find-file-prefix,boost/python.hpp,$(INCLUDE_SEARCH_PATHS))
CONF_P_INC=$(call find-file-prefix,Python.h,$(INCLUDE_SEARCH_PATHS))
CONF_B_LIB=$(call find-library,boost,$(LIB_SEARCH_PATH))
CONF_P_LIB=$(call find-library,python,$(LIB_SEARCH_PATH))
#CONF_P_USE=$(if $(CONF_B_INC),$(if $(CONF_P_INC),$(if $(CONF_B_LIB),$(if $(CONF_P_LIB),y,),),),)
CONF_P_USE=
CONF_TC_LIB=$(call find-library,tcmalloc,$(LIB_SEARCH_PATH))
CONF_UNAME_BIN=$(call find-program,uname)
CONF_UNAME_HARD=$(shell $(CONF_UNAME_BIN) -p)
CONF_UNAME_OS=$(shell $(CONF_UNAME_BIN) -s)
CONF_I386=$(if $(findstring i386,$(CONF_UNAME_HARD)),i386,$(if $(findstring i686,$(CONF_UNAME_HARD)),i386))
CONF_PPC=$(if $(findstring powerpc,$(CONF_UNAME_HARD)),ppc,)
CONF_UNAME_LINUX=$(findstring Linux,$(CONF_UNAME_OS))
CONF_UNAME_DARWIN=$(findstring Darwin,$(CONF_UNAME_OS))
CONF_PLATFORM_LINUX=$(if $(CONF_UNAME_LINUX),$(if $(CONF_I386),i386-linux-gcc),)
CONF_PLATFORM_DARWIN=$(if $(CONF_UNAME_DARWIN),$(if $(CONF_PPC),ppc-darwin-gcc),)
CONF_PLATFORM=$(if $(CONF_PLATFORM_LINUX),$(CONF_PLATFORM_LINUX),$(CONF_PLATFORM_DARWIN))
CONF_TCP=$(call is-dir,$(TOP_SRC_DIR)/src/tcp-bsd)
config.mk:
	@echo "Generating config.mk..."
	@$(call rm-f,config.mk)
	@echo CONFIGURED:=y >config.mk
    ifneq ($(CONF_P_USE),)
	@echo BOOST_PREFIX_INC:=$(CONF_B_INC) >>config.mk
	@echo PYTHON_PREFIX_INC:=$(CONF_P_INC) >>config.mk
	@echo PYTHON_USE:=y >>config.mk
    endif
    ifneq ($(CONF_TC_LIB),)
	@echo TC_LDFLAGS:=-L$(CONF_TC_LIB) -lpthread -ltcmalloc >>config.mk
    endif
	@echo PLATFORM:=$(CONF_PLATFORM) >>config.mk
    ifneq ($(CONF_TCP),)
	@echo TCP_USE=y >>config.mk
    endif

ifeq ($(CONFIGURED),y)

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
OPTI_FLAGS=-O0
FLAGS=-Wall -Werror -g3 $(OPTI_FLAGS)


LDFLAGS+=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)

PACKAGE_DIST:= \
	platform.mk \
	rules.mk \
	config.mk \
	Makefile \
	README \
	LICENSE \
	BUILD \
	NEWS \
	$(NULL)

# building of libyans.so
YANS_SRC:= \
	simulator/clock.cc \
	simulator/scheduler.cc \
	simulator/scheduler-list.cc \
	simulator/scheduler-heap.cc \
	simulator/event.cc \
	simulator/static-event.cc \
	simulator/cancellable-event.cc \
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
	src/common/seed-generator-mrg32k3a.cc \
	src/common/random-uniform-mrg32k3a.cc \
	src/common/rng-mrg32k3a.cc \
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
	src/arp/chunk-mac-llc-snap.cc \
	src/host/host.cc \
	src/host/network-interface.cc \
	src/host/loopback-interface.cc \
	src/ethernet/cable.cc \
	src/ethernet/chunk-mac-crc.cc \
	src/ethernet/chunk-mac-eth.cc \
	src/ethernet/ethernet-network-interface.cc \
	src/apps/udp-source.cc \
	src/apps/udp-sink.cc \
	src/apps/tcp-source.cc \
	src/apps/tcp-sink.cc \
	src/apps/periodic-generator.cc \
	src/apps/traffic-analyser.cc \
	src/80211/chunk-mac-80211-hdr.cc \
	src/80211/chunk-mac-80211-fcs.cc \
	src/80211/mac-stations.cc \
	src/80211/mac-station.cc \
	src/80211/arf-mac-stations.cc \
	src/80211/aarf-mac-stations.cc \
	src/80211/cr-mac-stations.cc \
	src/80211/ideal-mac-stations.cc \
	src/80211/propagation-model.cc \
	src/80211/channel-80211.cc \
	src/80211/transmission-mode.cc \
	src/80211/bpsk-mode.cc \
	src/80211/qam-mode.cc \
	src/80211/phy-80211.cc \
	src/80211/network-interface-80211-simple.cc \
	src/80211/network-interface-80211-simple-factory.cc \
	src/80211/mac-simple.cc \
	src/80211/mac-low.cc \
	src/80211/mac-parameters.cc \
	src/80211/dcf.cc \
	src/80211/mac-tx-middle.cc \
	src/80211/mac-rx-middle.cc \
	src/80211/dca-txop.cc \
	src/80211/mac-queue-80211e.cc \
	src/80211/mac-high-adhoc.cc \
	src/80211/network-interface-80211.cc \
	src/80211/network-interface-80211-factory.cc \
	test/test.cc \
	$(NULL)
YANS_HDR = \
	test/test.h \
	simulator/event.h \
	simulator/event.tcc \
	simulator/static-event.h \
	simulator/static-event.tcc \
	simulator/cancellable-event.h \
	simulator/cancellable-event.tcc \
	simulator/simulator.h \
	simulator/callback-event.tcc \
	simulator/clock.h \
	simulator/scheduler.h \
	simulator/scheduler-list.h \
	simulator/scheduler-heap.h \
	src/common/buffer.h \
	src/common/data-writer.h \
	src/common/pcap-writer.h \
	src/common/ipv4-address.h \
	src/common/population-analysis.h \
	src/common/utils.h \
	src/common/chunk-constant-data.h \
	src/common/mac-address.h \
	src/common/sgi-hashmap.h \
	src/common/chunk-fake-data.h \
	src/common/packet.h \
	src/common/tag-manager.h \
	src/common/chunk.h \
	src/common/packet-logger.h \
	src/common/trace-container.h \
	src/common/callback.tcc \
	src/common/ref-count.tcc \
	src/common/ui-traced-variable.tcc \
	src/common/si-traced-variable.tcc \
	src/common/f-traced-variable.tcc \
	src/common/seed-generator.h \
	src/common/random-uniform.h \
	src/common/rng-mrg32k3a.h \
	src/common/count-ptr-holder.tcc \
	src/ipv4/chunk-icmp.h \
	src/ipv4/defrag-state.h \
	src/ipv4/ipv4-route.h \
	src/ipv4/tcp-connection-listener.h \
	src/ipv4/chunk-ipv4.h \
	src/ipv4/ipv4-end-point.h \
	src/ipv4/tag-ipv4.h \
	src/ipv4/tcp.h \
	src/ipv4/chunk-tcp.h \
	src/ipv4/ipv4-end-points.h \
	src/ipv4/tcp-buffer.h \
	src/ipv4/chunk-udp.h \
	src/ipv4/ipv4.h \
	src/ipv4/tcp-connection.h \
	src/ipv4/udp.h \
	src/arp/arp-cache-entry.h \
	src/arp/arp.h \
	src/arp/chunk-arp.h \
	src/arp/chunk-mac-llc-snap.h \
	src/apps/periodic-generator.h \
	src/apps/tcp-source.h \
	src/apps/udp-sink.h \
	src/apps/tcp-sink.h \
	src/apps/traffic-analyser.h \
	src/apps/udp-source.h \
	src/80211/chunk-mac-80211-hdr.h \
	src/80211/chunk-mac-80211-fcs.h \
	src/80211/mac-stations.h \
	src/80211/mac-station.h \
	src/80211/arf-mac-stations.h \
	src/80211/aarf-mac-stations.h \
	src/80211/cr-mac-stations.h \
	src/80211/ideal-mac-stations.h \
	src/80211/propagation-model.h \
	src/80211/channel-80211.h \
	src/80211/transmission-mode.h \
	src/80211/bpsk-mode.h \
	src/80211/qam-mode.h \
	src/80211/phy-80211.h \
	src/80211/network-interface-80211-simple.h \
	src/80211/network-interface-80211-simple-factory.h \
	src/80211/mac-simple.h \
	src/80211/mac-low.h \
	src/80211/mac-parameters.h \
	src/80211/dcf.h \
	src/80211/mac-tx-middle.h \
	src/80211/mac-rx-middle.h \
	src/80211/mac-queue-80211e.h \
	src/80211/dca-txop.h \
	src/80211/mac-high-adhoc.h \
	src/80211/network-interface-80211.h \
	src/80211/network-interface-80211-factory.h \
	src/host/host.h \
	src/host/loopback-interface.h \
	src/host/network-interface.h \
	src/ethernet/cable.h \
	src/ethernet/chunk-mac-crc.h \
	src/ethernet/chunk-mac-eth.h \
	src/ethernet/ethernet-network-interface.h \
	src/thread/fiber-context.h \
	src/thread/fiber.h \
	src/thread/fiber-scheduler.h \
	src/thread/semaphore.h \
	src/thread/thread.h \
	$(NULL)
YANS_CXXFLAGS=$(CXXFLAGS) $(call gen-lib-build-flags)
YANS_CFLAGS=$(CFLAGS) $(call gen-lib-build-flags)
YANS_LDFLAGS=$(LDFLAGS) $(call gen-lib-link-flags)
YANS_OUTPUT=$(call gen-lib-name, yans)
ifeq ($(TCP_USE),y)
YANS_SRC += \
	src/ipv4/tcp-bsd/tcp-bsd-connection.cc
YANS_CXXFLAGS += -DTCP_USE_BSD
YANS_CXXFLAGS += -I$(TOP_SRC_DIR)/src/ipv4/tcp-bsd/
endif

# the benchmark for the simulator
BENCH_SRC=simulator/bench-simulator.cc
BENCH_OUTPUT=simulator/bench-simulator
BENCH_CXXFLAGS=$(CXXFLAGS)
BENCH_LDFLAGS=$(LDFLAGS) -lyans -L$(TOP_BUILD_DIR) $(TC_LDFLAGS)


# building of main-test
TEST_SRC=test/main-test.cc
TEST_OUTPUT=test/main-test
TEST_CXXFLAGS=$(CXXFLAGS)
TEST_LDFLAGS=$(LDFLAGS) -lyans -L$(TOP_BUILD_DIR) $(TC_LDFLAGS)

# building of sample applications
SAMPLE_CXX_SIMU_SRC= \
	samples/main-simulator.cc \
	$(NULL)
SAMPLE_CXX_SIMU_OUTPUT=samples/main-simulator
SAMPLE_CXX_SIMU_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMU_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_SIMU_FOR_SRC= \
	samples/main-forwarding-simulator.cc \
	$(NULL)
SAMPLE_CXX_SIMU_FOR_OUTPUT=samples/main-forwarding-simulator
SAMPLE_CXX_SIMU_FOR_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMU_FOR_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_SIMUTEMP_SRC= \
	samples/main-forwarding-simulator-template.cc \
	$(NULL)
SAMPLE_CXX_SIMUTEMP_OUTPUT=samples/main-forwarding-simulator-template
SAMPLE_CXX_SIMUTEMP_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMUTEMP_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_SIMPLE_SRC= \
	samples/main-simple.cc \
	$(NULL)
SAMPLE_CXX_SIMPLE_OUTPUT=samples/main-simple
SAMPLE_CXX_SIMPLE_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_SIMPLE_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_ROUTER_SRC= \
	samples/main-router.cc \
	$(NULL)
SAMPLE_CXX_ROUTER_OUTPUT=samples/main-router
SAMPLE_CXX_ROUTER_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_ROUTER_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_TCP_SRC= \
	samples/main-tcp.cc \
	$(NULL)
SAMPLE_CXX_TCP_OUTPUT=samples/main-tcp
SAMPLE_CXX_TCP_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_TCP_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_THREAD_SRC= \
	samples/main-thread.cc \
	$(NULL)
SAMPLE_CXX_THREAD_OUTPUT=samples/main-thread
SAMPLE_CXX_THREAD_CXXFLAGS=$(CXXFLAGS)
SAMPLE_CXX_THREAD_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_80211_SIMPLE_SRC= \
	samples/main-80211-simple.cc \
	$(NULL)
SAMPLE_CXX_80211_SIMPLE_OUTPUT=samples/main-80211-simple
SAMPLE_CXX_80211_SIMPLE_CXXFLAGS=$(CXXFLAGS) -I$(TOP_SRC_DIR)/src/80211
SAMPLE_CXX_80211_SIMPLE_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_80211_ADHOC_SRC= \
	samples/main-80211-adhoc.cc \
	$(NULL)
SAMPLE_CXX_80211_ADHOC_OUTPUT=samples/main-80211-adhoc
SAMPLE_CXX_80211_ADHOC_CXXFLAGS=$(CXXFLAGS) -I$(TOP_SRC_DIR)/src/80211
SAMPLE_CXX_80211_ADHOC_LDFLAGS=$(LDFLAGS) -L$(TOP_BUILD_DIR) -lyans $(TC_LDFLAGS)




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
	python/export-tcp-source.cc \
	python/export-tcp-sink.cc \
	python/test-periodic-generator.py \
	samples/test-simulator.py \
	samples/test-thread.py \
	samples/test-simulator-gc.py \
	samples/test-simple.py \
	$(NULL)
YANS_PYTHON_HDR= \
	python/function-holder.h \
	python/export-callback.tcc \
	python/export-callback-traits.tcc \
	$(NULL)
YANS_PYTHON_OUTPUT=python/$(call gen-pymod-name, _yans)
YANS_PYTHON_CXXFLAGS=$(CXXFLAGS) $(call gen-pymod-build-flags)
YANS_PYTHON_LDFLAGS=$(LDFLAGS) $(call gen-pymod-link-flags) -lyans -L$(TOP_BUILD_DIR)


ALL:= \
	YANS \
	BENCH \
	TEST \
	SAMPLE_CXX_SIMPLE \
	SAMPLE_CXX_ROUTER \
	SAMPLE_CXX_TCP \
	SAMPLE_CXX_SIMU \
	SAMPLE_CXX_SIMU_FOR \
	SAMPLE_CXX_SIMUTEMP \
	SAMPLE_CXX_THREAD \
	SAMPLE_CXX_80211_SIMPLE \
	SAMPLE_CXX_80211_ADHOC \
	$(NULL)

ifeq ($(PYTHON_USE),y)
ALL += 	YANS_PYTHON
endif


run-opti-arc-profile-hook:
	LD_LIBRARY_PATH=bin/opti-arc $(TOP_BUILD_DIR)/opti-arc/samples/main-simple --slow --heap

endif #CONFIGURED

include ./rules-end.mk
