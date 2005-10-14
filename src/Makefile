RUN_TESTS=-DRUN_SELF_TESTS=1
INCLUDES=-I./fiber -I./simulator -I./ -I./file -I./arp -I./common -I./ipv4
CXXFLAGS=-Wall -Werror -O0 -gdwarf-2 $(RUN_TESTS) $(INCLUDES)

SOURCE= \
	chunk-tcp.cc \
	chunk-udp.cc \
	chunk-data.cc \
	mac-address.cc \
	network-interface.cc \
	loopback-interface.cc \
	transport-protocol.cc \
	udp.cc \
	host.cc \
	thread.cc \
	process.cc \
	socket-udp.cc \
	common/buffer.cc \
	common/chunk.cc \
	common/tag-manager.cc \
	common/packet.cc \
	common/utils.cc \
	ipv4/chunk-ipv4.cc \
	ipv4/ipv4-address.cc \
	ipv4/tag-ipv4.cc \
	ipv4/ipv4-route.cc \
	ipv4/ipv4.cc \
	file/read-file.cc \
	file/write-file.cc \
	fiber/fiber-stack-ia32.cc \
	fiber/fiber-context-posix.cc \
	fiber/fiber.cc \
	fiber/fiber-scheduler.cc \
	fiber/semaphore.cc \
	fiber/runnable.cc \
	simulator/clock.cc \
	simulator/event-heap.cc \
	simulator/event-runnable.cc \
	simulator/simulator.cc \
	simulator/event.cc \
	ethernet/ethernet-network-interface.cc \
	ethernet/cable.cc \
	ethernet/chunk-mac-llc-snap.cc \
	ethernet/chunk-mac-crc.cc \
	arp/arp.cc \
	arp/arp-cache-entry.cc \
	arp/chunk-arp.cc \
	test.cc \
	main.cc

OBJFILES=$(addsuffix .o, $(basename $(SOURCE)))

all: test

test: $(OBJFILES)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^


clean:
	rm -f $(OBJFILES) *~ test;
