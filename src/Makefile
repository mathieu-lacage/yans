CXXFLAGS=-Wall -Werror -O3

SOURCE= \
	buffer.cc \
	chunk.cc \
	tag-manager.cc \
	tag-ipv4.cc \
	packet.cc \
	utils.cc \
	chunk-ipv4.cc \
	chunk-tcp.cc \
	chunk-udp.cc \
	mac-address.cc \
	ipv4-address.cc \
	network-interface.cc \
	loopback-interface.cc \
	ipv4-route.cc \
	ipv4.cc \
	host.cc \
	socket-udp.cc \
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
