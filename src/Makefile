SIMULATOR=simple

RUN_SELF_TESTS=-DRUN_SELF_TESTS=1
INCLUDES=-I./fiber -I./simulator -I./ -I./arp -I./common -I./ipv4 -I./ethernet -I./test -I./host -I./os-model
DEFINES=$(RUN_SELF_TESTS)
FLAGS=-Wall -Werror -O0 -gdwarf-2

MAIN_TEST_SOURCE=test/main-test.cc
MAIN_SIMPLE_SOURCE=samples/main-simple.cc
MAIN_ROUTER_SOURCE=samples/main-router.cc
SOURCE= \
	test/test.cc \
	samples/chunk-fake-data.cc \
	samples/udp-source.cc \
	samples/udp-sink.cc \
	host/mac-address.cc \
	host/network-interface.cc \
	host/loopback-interface.cc \
	host/transport-protocol.cc \
	host/host.cc \
	host/host-tracer.cc \
	host/network-interface-tracer.cc \
	os-model/write-file.cc \
	common/buffer.cc \
	common/chunk.cc \
	common/tag-manager.cc \
	common/packet.cc \
	common/utils.cc \
	ipv4/udp.cc \
	ipv4/chunk-tcp.cc \
	ipv4/chunk-udp.cc \
	ipv4/chunk-ipv4.cc \
	ipv4/chunk-piece.cc \
	ipv4/chunk-icmp.cc \
	ipv4/ipv4-endpoint.cc \
	ipv4/ipv4-address.cc \
	ipv4/tag-ipv4.cc \
	ipv4/ipv4-route.cc \
	ipv4/defrag-state.cc \
	ipv4/ipv4.cc \
	simulator/clock.cc \
	simulator/event-heap.cc \
	simulator/event.cc \
	ethernet/ethernet-network-interface.cc \
	ethernet/cable.cc \
	ethernet/chunk-mac-llc-snap.cc \
	ethernet/chunk-mac-crc.cc \
	arp/arp.cc \
	arp/arp-cache-entry.cc \
	arp/chunk-arp.cc

ifeq ($(SIMULATOR),fiber)
SOURCE += \
	simulator/event-runnable.cc \
	simulator/simulator-fiber.cc \
	fiber/fiber-context-x86.c \
	fiber/fiber.cc \
	fiber/fiber-scheduler.cc \
	fiber/semaphore.cc \
	fiber/runnable.cc \
	os-model/chunk-data.cc \
	os-model/thread.cc \
	os-model/process.cc \
	os-model/socket-udp.cc \
	os-model/read-file.cc 
DEFINES += -DSIMULATOR_FIBER
endif

ifeq ($(SIMULATOR),simple)
SOURCE += \
	simulator/simulator-simple.cc
DEFINES += -DSIMULATOR_SIMPLE
endif


CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)

OBJFILES=$(addsuffix .o, $(basename $(SOURCE)))
OBJ_MAIN_TEST=$(addsuffix .o, $(basename $(MAIN_TEST_SOURCE)))
OBJ_MAIN_SIMPLE=$(addsuffix .o, $(basename $(MAIN_SIMPLE_SOURCE)))
OBJ_MAIN_ROUTER=$(addsuffix .o, $(basename $(MAIN_ROUTER_SOURCE)))

all: test/main-test samples/main-simple samples/main-router

test/main-test: $(OBJFILES) $(OBJ_MAIN_TEST)
	$(CXX) $(LDFLAGS) -o $@ $^

samples/main-simple: $(OBJFILES) $(OBJ_MAIN_SIMPLE)
	$(CXX) $(LDFLAGS) -o $@ $^
samples/main-router: $(OBJFILES) $(OBJ_MAIN_ROUTER)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^


clean:
	rm -f $(OBJFILES) test/main-test samples/main-simple \
	samples/main-router $(OBJ_MAIN_TEST) $(OBJ_MAIN_SIMPLE) \
	$(OBJ_MAIN_ROUTER) \
	*~ arp/*~ ethernet/*~ simulator/*~ fiber/*~ ipv4/*~ \
	common/*~ os-model/*~ host/*~ samples/*~ test/*~ ;
