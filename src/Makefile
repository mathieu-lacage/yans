RUN_TESTS=-DRUN_SELF_TESTS=1
CXXFLAGS=-Wall -Werror -O0 -gdwarf-2 $(RUN_TESTS)

SOURCE= \
	buffer.cc \
	chunk.cc \
	chunk-ipv4.cc \
	chunk-tcp.cc \
	chunk-udp.cc \
	chunk-data.cc \
	tag-manager.cc \
	tag-ipv4.cc \
	packet.cc \
	utils.cc \
	mac-address.cc \
	ipv4-address.cc \
	network-interface.cc \
	loopback-interface.cc \
	ipv4-route.cc \
	ipv4.cc \
	transport-protocol.cc \
	udp.cc \
	host.cc \
	socket-udp.cc \
	fiber-stack-ia32.cc \
	fiber-context-posix.cc \
	fiber.cc \
	fiber-scheduler.cc \
	clock.cc \
	event-heap.cc \
	event-fiber.cc \
	simulator.cc \
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
