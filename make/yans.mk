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
YANS_HDR = \
	test/test.h \
	simulator/event.h \
	simulator/event.tcc \
	simulator/simulator.h \
	simulator/callback-event.tcc \
	simulator/clock.h \
	simulator/event-heap.h \
	src/common/buffer.h \
	src/common/data-writer.h \
	src/common/pcap-writer.h \
	src/common/traced-variable-test.h \
	src/common/callback-test.h \
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
	src/apps/periodic-generator.h \
	src/apps/tcp-source.h \
	src/apps/udp-sink.h \
	src/apps/tcp-sink.h \
	src/apps/traffic-analyser.h \
	src/apps/udp-source.h \
	src/host/host.h \
	src/host/loopback-interface.h \
	src/host/network-interface.h \
	src/ethernet/cable.h \
	src/ethernet/chunk-mac-crc.h \
	src/ethernet/chunk-mac-llc-snap.h \
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
YANS_OUTPUT=$(TOP_BUILD_DIR)/$(call gen-lib-name, yans)
ifeq ($(TCP),bsd)
YANS_SRC += \
	src/ipv4/tcp-bsd/tcp-bsd-connection.cc
YANS_CXXFLAGS += -DTCP_USE_BSD
YANS_CXXFLAGS += -I$(TOP_SRC_DIR)/src/ipv4/tcp-bsd/
endif

ALL += \
	YANS \
	$(NULL)
