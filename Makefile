# The main Makefile is supposed to store in the ALL variable
# the list of all the project names to build. Each project
# whose name is PROJECT_NAME can define the variables:
#   - PROJECT_NAME_SRC
#   - PROJECT_NAME_HDR
#   - PROJECT_NAME_INST_HDR
#   - PROJECT_NAME_NAME
#   - PROJECT_NAME_TYPE
#   - PROJECT_NAME_OUTPUT_DIR
#   - PROJECT_NAME_CFLAGS
#   - PROJECT_NAME_CXXFLAGS
#   - PROJECT_NAME_ASFLAGS
# Furthermore, for each .c/.cc/.s file stored in a _SRC variable
# you can define the variables:
#   - filename_CFLAGS
#   - filename_CXXFLAGS
#   - filename_ASFLAGS
# _TYPE is one of:
#   - shared-library
#   - python-module
#   - python-cxx-module
#   - python-executable
#   - executable
# _NAME:
#   - if _TYPE is shared-library, _NAME is used to generate the name
#     of the final shared library binary file. On linux, the final
#     binary file is libFOO_NAME.so and it is installed in 
#     TOP_BUILD_DIR/lib. _NAME is also used to generate the
#     name of the directory which contains the headers listed in 
#     INST_HDR. i.e., the headers are installed in 
#     TOP_BUILD_DIR/include/FOO_NAME.
#   - if _TYPE is python-cxx-module, _NAME is used to generate the
#     name of the final python module which is installed in 
#     TOP_BUILD_DIR/lib/python.

include ./rules-start.mk
-include ./config.mk

TOP:=.
NULL:=
PACKAGE_NAME:=yans
PACKAGE_VERSION:=0.9.0
TOP_BUILD_DIR:=$(TOP)/build-dir
TOP_SRC_DIR:=$(TOP)

ifeq ($(PLATFORM),)

INCLUDE_SEARCH_PATHS:= \
 /usr/include \
 /include \
 /usr/local/include \
 /usr/include/python2.4 \
 /usr/include/python2.3 \
 $(NULL)
LIB_SEARCH_PATH:= \
 /usr/lib \
 /lib \
 /usr/local/lib \
 $(NULL)
CONF_B_INC=$(call find-file-prefix,boost/python.hpp,$(INCLUDE_SEARCH_PATHS))
CONF_P_INC=$(call find-file-prefix,Python.h,$(INCLUDE_SEARCH_PATHS))
CONF_B_LIB=$(call find-library,boost_python,$(LIB_SEARCH_PATH))
CONF_P_LIB=$(call find-one-library,python2.4 python2.3,$(LIB_SEARCH_PATH))
CONF_P_USE=$(if $(CONF_B_INC),$(if $(CONF_P_INC),$(if $(CONF_B_LIB),$(if $(CONF_P_LIB),y,),),),)
#CONF_P_USE=
CONF_TC_LIB=$(call find-library,tcmalloc,$(LIB_SEARCH_PATH))
CONF_TCP=$(call is-dir,$(TOP_SRC_DIR)/src/tcp-bsd)
config.mk:
	@echo "Generating config.mk..."
	@$(call rm-f,config.mk)
	@echo PLATFORM:=$(call guess-platform) >config.mk
	@echo BOOST_PREFIX_INC:=$(CONF_B_INC) >>config.mk
	@echo PYTHON_PREFIX_INC:=$(CONF_P_INC) >>config.mk
    ifneq ($(CONF_P_USE),)
	@echo PYTHON_USE:=y >>config.mk
    else
	@echo PYTHON_USE:= \# set it to \"y\" to enable python >>config.mk
    endif
    ifneq ($(CONF_TC_LIB),)
	@echo TC_LDFLAGS:=-L$(CONF_TC_LIB) -lpthread -ltcmalloc >>config.mk
    else
	@echo TC_LDFLAGS:= \# cflags for the google malloc library included in the google perf tools >>config.mk
    endif
    ifneq ($(CONF_TCP),)
	@echo TCP_USE=y >>config.mk
    else
	@echo TCP_USE= \# set it to \"y\" to enable the tcp code >>config.mk
    endif

else # PLATFORM

DEFINES:=-DRUN_SELF_TESTS=1
INCLUDES:=
OPTI_FLAGS:=-O0
FLAGS:=-Wall -Werror -g3 $(OPTI_FLAGS)


LDFLAGS+=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)

PACKAGE_DIST:= \
	rules-start.mk \
	rules-end.mk \
	Makefile \
	README \
	LICENSE \
	BUILD \
	NEWS \
	$(NULL)

ifeq ($(PLATFORM), i386-linux-gcc)
FIBER_CONTEXT_PLATFORM:= \
	src/thread/fiber-context-i386-linux-gcc.c \
	$(NULL)
endif
ifeq ($(PLATFORM), ppc-darwin-gcc)
FIBER_CONTEXT_PLATFORM:= \
	src/thread/fiber-context-ppc-darwin-gcc.c \
	src/thread/ppc-darwin-gcc-switch.s \
	$(NULL)
/src/thread/ppc-darwin-gcc-switch.s_ASFLAGS:=-arch ppc -g
endif

# building of libyans.so
YANS_SRC:= \
	src/simulator/scheduler.cc \
	src/simulator/scheduler-list.cc \
	src/simulator/scheduler-heap.cc \
	src/simulator/scheduler-map.cc \
	src/simulator/event.cc \
	src/simulator/static-event.cc \
	src/simulator/cancellable-event.cc \
	src/simulator/simulator.cc \
	$(FIBER_CONTEXT_PLATFORM) \
	src/thread/semaphore.cc \
	src/thread/fiber.cc \
	src/thread/fiber-scheduler.cc \
	src/thread/thread.cc \
	src/common/wall-clock-ms-unix.cc \
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
	src/common/reference-list-test.cc \
	src/common/traced-variable-test.cc \
	src/common/packet-logger.cc \
	src/common/trace-container.cc \
	src/common/pcap-writer.cc \
	src/common/seed-generator-mrg32k3a.cc \
	src/common/random-uniform-mrg32k3a.cc \
	src/common/rng-mrg32k3a.cc \
	src/common/timeout.cc \
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
	src/80211/ssid.cc \
	src/80211/supported-rates.cc \
	src/80211/capability-information.cc \
	src/80211/status-code.cc \
	src/80211/chunk-mgt.cc \
	src/80211/mac-high-nqsta.cc \
	src/80211/mac-high-nqap.cc \
	src/80211/network-interface-80211.cc \
	src/80211/network-interface-80211-factory.cc \
	src/test/test.cc \
	$(NULL)
YANS_INST_HDR := \
	src/test/test.h \
	src/simulator/event.h \
	src/simulator/event.tcc \
	src/simulator/static-event.h \
	src/simulator/static-event.tcc \
	src/simulator/cancellable-event.h \
	src/simulator/cancellable-event.tcc \
	src/simulator/simulator.h \
	src/common/wall-clock-ms.h \
	src/common/buffer.h \
	src/common/data-writer.h \
	src/common/pcap-writer.h \
	src/common/ipv4-address.h \
	src/common/population-analysis.h \
	src/common/utils.h \
	src/common/chunk-constant-data.h \
	src/common/mac-address.h \
	src/common/chunk-fake-data.h \
	src/common/packet.h \
	src/common/tag-manager.h \
	src/common/chunk.h \
	src/common/packet-logger.h \
	src/common/trace-container.h \
	src/common/callback.h \
	src/common/reference-list.h \
	src/common/ui-traced-variable.tcc \
	src/common/si-traced-variable.tcc \
	src/common/f-traced-variable.tcc \
	src/common/random-uniform.h \
	src/common/count-ptr-holder.tcc \
	src/common/timeout.h \
	src/ipv4/ipv4-route.h \
	src/apps/periodic-generator.h \
	src/apps/tcp-source.h \
	src/apps/udp-sink.h \
	src/apps/tcp-sink.h \
	src/apps/traffic-analyser.h \
	src/apps/udp-source.h \
	src/80211/network-interface-80211-simple.h \
	src/80211/network-interface-80211-simple-factory.h \
	src/80211/network-interface-80211.h \
	src/80211/network-interface-80211-factory.h \
	src/80211/channel-80211.h \
	src/80211/ssid.h \
	src/host/host.h \
	src/host/network-interface.h \
	src/ethernet/cable.h \
	src/ethernet/ethernet-network-interface.h \
	src/thread/semaphore.h \
	src/thread/thread.h \
	$(NULL)
YANS_HDR := \
	src/simulator/clock.h \
	src/simulator/scheduler.h \
	src/simulator/scheduler-list.h \
	src/simulator/scheduler-heap.h \
	src/simulator/scheduler-map.h \
	src/common/sgi-hashmap.h \
	src/common/ref-count.tcc \
	src/common/seed-generator.h \
	src/common/rng-mrg32k3a.h \
	src/ipv4/chunk-icmp.h \
	src/ipv4/defrag-state.h \
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
	src/80211/chunk-mac-80211-hdr.h \
	src/80211/chunk-mac-80211-fcs.h \
	src/80211/mac-stations.h \
	src/80211/mac-station.h \
	src/80211/arf-mac-stations.h \
	src/80211/aarf-mac-stations.h \
	src/80211/cr-mac-stations.h \
	src/80211/ideal-mac-stations.h \
	src/80211/propagation-model.h \
	src/80211/transmission-mode.h \
	src/80211/bpsk-mode.h \
	src/80211/qam-mode.h \
	src/80211/phy-80211.h \
	src/80211/mac-simple.h \
	src/80211/mac-low.h \
	src/80211/mac-parameters.h \
	src/80211/dcf.h \
	src/80211/mac-tx-middle.h \
	src/80211/mac-rx-middle.h \
	src/80211/mac-queue-80211e.h \
	src/80211/dca-txop.h \
	src/80211/mac-high-adhoc.h \
	src/80211/supported-rates.h \
	src/80211/capability-information.h \
	src/80211/status-code.h \
	src/80211/chunk-mgt.h \
	src/80211/mac-high-nqsta.h \
	src/80211/mac-high-nqap.h \
	src/host/loopback-interface.h \
	src/ethernet/chunk-mac-crc.h \
	src/ethernet/chunk-mac-eth.h \
	src/thread/fiber-context.h \
	src/thread/fiber.h \
	src/thread/fiber-scheduler.h \
	$(NULL)
YANS_INCLUDES:=\
 -I$(TOP_SRC_DIR)/src/simulator \
 -I$(TOP_SRC_DIR)/src/common \
 -I$(TOP_SRC_DIR)/src/test \
 -I$(TOP_SRC_DIR)/src/host \
 -I$(TOP_SRC_DIR)/src/ipv4 \
 -I$(TOP_SRC_DIR)/src/arp \
 $(NULL)
YANS_CXXFLAGS:=$(CXXFLAGS) $(YANS_INCLUDES) -DDISABLE_IPV4_CHECKSUM=1
YANS_CFLAGS:=$(CFLAGS) $(YANS_INCLUDES) -DDISABLE_IPV4_CHECKSUM=1
YANS_LDFLAGS:=$(LDFLAGS)
YANS_NAME:=yans
YANS_TYPE:=shared-library
YANS_OUTPUT_DIR:=
ifeq ($(TCP_USE),y)
YANS_SRC += \
	src/ipv4/tcp-bsd/tcp-bsd-connection.cc
YANS_CXXFLAGS += -DTCP_USE_BSD
YANS_CXXFLAGS += -I$(TOP_SRC_DIR)/src/ipv4/tcp-bsd/
endif

REPLAY_SIMULATION_SRC:=utils/replay-simulation.cc
REPLAY_SIMULATION_NAME:=replay-simulation
REPLAY_SIMULATION_TYPE:=executable
REPLAY_SIMULATION_CXXFLAGS:=$(CXXFLAGS)
REPLAY_SIMULATION_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

# the benchmark for the simulator
BENCH_SRC:=utils/bench-simulator.cc
BENCH_NAME:=bench-simulator
BENCH_OUTPUT_DIR:=
BENCH_TYPE:=executable
BENCH_CXXFLAGS:=$(CXXFLAGS)
BENCH_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)


# building of main-test
TEST_SRC:=src/test/main-test.cc
TEST_NAME:=test
TEST_OUTPUT_DIR:=
TEST_TYPE:=executable
TEST_CXXFLAGS:=$(CXXFLAGS)
TEST_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

# building of sample applications
SAMPLE_CXX_SIMU_SRC:= \
	samples/main-simulator.cc \
	$(NULL)
SAMPLE_CXX_SIMU_NAME:=main-simulator
SAMPLE_CXX_SIMU_OUTPUT_DIR:=
SAMPLE_CXX_SIMU_TYPE:=executable
SAMPLE_CXX_SIMU_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_SIMU_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_SIMU_FOR_SRC:= \
	samples/main-forwarding-simulator.cc \
	$(NULL)
SAMPLE_CXX_SIMU_FOR_NAME:=main-forwarding-simulator
SAMPLE_CXX_SIMU_FOR_OUTPUT_DIR:=
SAMPLE_CXX_SIMU_FOR_TYPE:=executable
SAMPLE_CXX_SIMU_FOR_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_SIMU_FOR_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_SIMUTEMP_SRC:= \
	samples/main-forwarding-simulator-template.cc \
	$(NULL)
SAMPLE_CXX_SIMUTEMP_NAME:=main-forwarding-simulator-template
SAMPLE_CXX_SIMUTEMP_OUTPUT_DIR:=
SAMPLE_CXX_SIMUTEMP_TYPE:=executable
SAMPLE_CXX_SIMUTEMP_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_SIMUTEMP_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_SIMPLE_SRC:= \
	samples/main-simple.cc \
	$(NULL)
SAMPLE_CXX_SIMPLE_NAME:=main-simple
SAMPLE_CXX_SIMPLE_OUTPUT_DIR:=
SAMPLE_CXX_SIMPLE_TYPE:=executable
SAMPLE_CXX_SIMPLE_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_SIMPLE_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_ROUTER_SRC:= \
	samples/main-router.cc \
	$(NULL)
SAMPLE_CXX_ROUTER_NAME:=main-router
SAMPLE_CXX_ROUTER_OUTPUT_DIR:=
SAMPLE_CXX_ROUTER_TYPE:=executable
SAMPLE_CXX_ROUTER_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_ROUTER_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_TCP_SRC:= \
	samples/main-tcp.cc \
	$(NULL)
SAMPLE_CXX_TCP_NAME:=main-tcp
SAMPLE_CXX_TCP_OUTPUT_DIR:=
SAMPLE_CXX_TCP_TYPE:=executable
SAMPLE_CXX_TCP_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_TCP_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_THREAD_SRC:= \
	samples/main-thread.cc \
	$(NULL)
SAMPLE_CXX_THREAD_NAME:=main-thread
SAMPLE_CXX_THREAD_OUTPUT_DIR:=
SAMPLE_CXX_THREAD_TYPE:=executable
SAMPLE_CXX_THREAD_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_THREAD_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_80211_SIMPLE_SRC:= \
	samples/main-80211-simple.cc \
	$(NULL)
SAMPLE_CXX_80211_SIMPLE_NAME:=main-80211-simple
SAMPLE_CXX_80211_SIMPLE_OUTPUT_DIR:=
SAMPLE_CXX_80211_SIMPLE_TYPE:=executable
SAMPLE_CXX_80211_SIMPLE_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_80211_SIMPLE_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)

SAMPLE_CXX_80211_ADHOC_SRC:= \
	samples/main-80211-adhoc.cc \
	$(NULL)
SAMPLE_CXX_80211_ADHOC_NAME:=main-80211-adhoc
SAMPLE_CXX_80211_ADHOC_OUTPUT_DIR:=
SAMPLE_CXX_80211_ADHOC_TYPE:=executable
SAMPLE_CXX_80211_ADHOC_CXXFLAGS:=$(CXXFLAGS)
SAMPLE_CXX_80211_ADHOC_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)




# building of python bindings.
YANS_PYTHON_CXX_SRC:= \
	python/yans-simulator.cc \
	python/export-simulator.cc \
	python/function-holder.cc \
	python/export-thread.cc \
	python/export-cpp-callback-factory.cc \
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
	$(NULL)
YANS_PYTHON_CXX_HDR:= \
	python/function-holder.h \
	python/export-callback.tcc \
	python/export-callback-traits.tcc \
	$(NULL)
YANS_PYTHON_CXX_NAME:=_yans
YANS_PYTHON_CXX_TYPE:=python-cxx-module
YANS_PYTHON_CXX_CXXFLAGS:=$(CXXFLAGS)
YANS_PYTHON_CXX_LDFLAGS:=$(LDFLAGS) -lyans

YANS_PYTHON_SRC:=\
	python/yans/__init__.py \
	$(NULL)
YANS_PYTHON_NAME:=yans
YANS_PYTHON_TYPE:=python-module

YANS_PYTHON_SIMULATOR_SRC:=\
	python/yans/simulator/__init__.py \
	$(NULL)
YANS_PYTHON_SIMULATOR_NAME:=yans.simulator
YANS_PYTHON_SIMULATOR_TYPE:=python-module

YANS_PYTHON_TEST_PERIO_SRC:=\
	samples/test-periodic-generator.py \
	$(NULL)
YANS_PYTHON_TEST_PERIO_NAME:=pytest-periodic-generator
YANS_PYTHON_TEST_PERIO_TYPE:=python-executable

YANS_CPP_TEST_PERIO_SRC:=\
	samples/test-periodic-generator.cc \
	$(NULL)
YANS_CPP_TEST_PERIO_CXXFLAGS:=$(CXXFLAGS)
YANS_CPP_TEST_PERIO_LDFLAGS:=$(LDFLAGS) -lyans $(TC_LDFLAGS)
YANS_CPP_TEST_PERIO_NAME:=cpptest-periodic-generator
YANS_CPP_TEST_PERIO_TYPE:=executable


YANS_PYTHON_SAMPLE_SIMU_SRC:=\
	samples/test-simulator.py \
	$(NULL)
YANS_PYTHON_SAMPLE_SIMU_NAME:=pytest-simulator
YANS_PYTHON_SAMPLE_SIMU_TYPE:=python-executable

YANS_PYTHON_SAMPLE_THREAD_SRC:=\
	samples/test-thread.py \
	$(NULL)
YANS_PYTHON_SAMPLE_THREAD_NAME:=pytest-thread
YANS_PYTHON_SAMPLE_THREAD_TYPE:=python-executable

YANS_PYTHON_SAMPLE_SIMUGC_SRC:=\
	samples/test-simulator-gc.py \
	$(NULL)
YANS_PYTHON_SAMPLE_SIMUGC_NAME:=pytest-simu-gc
YANS_PYTHON_SAMPLE_SIMUGC_TYPE:=python-executable

YANS_PYTHON_SAMPLE_SIMPLE_SRC:=\
	samples/test-simple.py \
	$(NULL)
YANS_PYTHON_SAMPLE_SIMPLE_NAME:=pytest-simple
YANS_PYTHON_SAMPLE_SIMPLE_TYPE:=python-executable

ALL:= \
	YANS \
	REPLAY_SIMULATION \
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
	YANS_CPP_TEST_PERIO \
	$(NULL)

ifeq ($(PYTHON_USE),y)
ALL += \
	YANS_PYTHON_CXX \
	YANS_PYTHON \
	YANS_PYTHON_TEST_PERIO \
	YANS_PYTHON_SIMULATOR \
	YANS_PYTHON_SAMPLE_SIMU \
	YANS_PYTHON_SAMPLE_THREAD \
	YANS_PYTHON_SAMPLE_SIMUGC \
	YANS_PYTHON_SAMPLE_SIMPLE \
	$(NULL)
endif


run-opti-arc-profile-hook:
	LD_LIBRARY_PATH=bin/opti-arc $(TOP_BUILD_DIR)/opti-arc/samples/main-simple --slow --heap

endif #CONFIGURED

include ./rules-end.mk
