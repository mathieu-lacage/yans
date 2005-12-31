TOP=.
INSTALL=bin
NULL=
DEFINES=
INCLUDES=-I./simulator
FLAGS=-Wall -Werror -O0 -gdwarf-2
LDFLAGS=

CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)

YANSSRC= \
	$(TOP)/simulator/clock.cc \
	$(TOP)/simulator/event-heap.cc \
	$(TOP)/simulator/event.cc \
	$(TOP)/simulator/simulator-simple.cc \
	$(NULL)

YANSPYTHONSRC= \
	$(TOP)/python/yans.cc \
	$(TOP)/python/export-simulator.cc \
	$(TOP)/python/export-event.cc \
	$(NULL)

YANSOBJ=$(addsuffix .o, $(basename $(YANSSRC)))
LIBYANS=$(TOP)/libyans.so
YANSPYTHONOBJ=$(addsuffix .o, $(basename $(YANSPYTHONSRC)))
LIBYANSPYTHON=$(TOP)/python/yansmodule.so

all: $(LIBYANS) $(LIBYANSPYTHON)

install: all $(INSTALL)
	cp $(LIBYANS) $(INSTALL);
	cp $(LIBYANSPYTHON) $(INSTALL);

$(INSTALL):
	mkdir -p $@;


.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

$(LIBYANS): $(YANSOBJ)
	$(CXX) $(LDFLAGS) -shared -o $@ $^

$(YANSPYTHONOBJ): CXXFLAGS+=-I/usr/include/python2.4
$(LIBYANSPYTHON): $(YANSPYTHONOBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP) -lyans -shared -o $@ $^

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

TILDES=$(addsuffix ~, $(YANSSRC))
TILDES+=$(addsuffix ~, $(YANSPYTHONSRC))

clean:
	rm -f $(YANSOBJ) $(LIBYANS) \
	$(YANSPYTHONOBJ) $(LIBYANSPYTHON) \
	$(TILDES) \
	2>/dev/null;
