CXXFLAGS=-Wall -Werror -O3
LDFLAGS=-O3

all: test

test: main.o object.o test-object.o time.o
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o test *~ 2>/dev/null
