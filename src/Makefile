RUN_TESTS=-DRUN_SELF_TESTS=1
INCLUDES=-I./fiber -I./simulator -I./
CXXFLAGS=-Wall -Werror -O0 -gdwarf-2 $(RUN_TESTS) $(INCLUDES)

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
	thread.cc \
	socket-udp.cc \
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
