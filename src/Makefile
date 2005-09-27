CXXFLAGS=-Wall -Werror -O3

SOURCE= \
	buffer.cc \
	chunk.cc \
	packet.cc \
	test.cc

OBJFILES=$(addsuffix .o, $(basename $(SOURCE)))

all: test

test: $(OBJFILES)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.cc
	$(CC) $(CXXFLAGS) -c -o $@ $^


clean:
	rm -f $(OBJFILES) *~ test;
