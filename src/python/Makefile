
INCLUDES=-I/usr/include/python2.4 -I../host -I../ipv4
CXXFLAGS= -gdwarf-2 -O0 -Wall -Werror $(INCLUDES)
LDFLAGS= -lboost_python

SOURCE= \
 host.cc

OBJFILES = $(addsuffix .o, $(basename $(SOURCE)))

all: yansmodule.so

yansmodule.so: $(OBJFILES)
	$(CXX) -shared $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o *.so *~ 2>/dev/null
