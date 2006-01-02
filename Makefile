TOP=.
TOP_INSTALL=bin
TOP_PYTHON_INSTALL=$(TOP_INSTALL)/yans
NULL=
DEFINES=
INCLUDES=-I./simulator
FLAGS=-Wall -Werror -O0 -gdwarf-2
LDFLAGS=
INSTALL_DIRS= \
	$(TOP_INSTALL) \
	$(TOP_PYTHON_INSTALL) \
	$(TOP_PYTHON_INSTALL)/simulator \
	$(NULL)

CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)

YANS_SRC= \
	$(TOP)/simulator/clock.cc \
	$(TOP)/simulator/event-heap.cc \
	$(TOP)/simulator/event.cc \
	$(TOP)/simulator/simulator-simple.cc \
	$(TOP)/src/thread/fiber-context-x86-linux-gcc.cc \
	$(TOP)/src/thread/semaphore.cc \
	$(TOP)/src/thread/thread.cc \
	$(TOP)/src/thread/fiber.cc \
	$(NULL)

SIMULATOR_PYTHON_SRC= \
	$(TOP)/python/yans-simulator.cc \
	$(TOP)/python/export-simulator.cc \
	$(TOP)/python/export-event.cc \
	$(NULL)
YANS_PYTHON_SRC= \
	$(SIMULATOR_PYTHON_SRC) \
	$(NULL)

YANS_OBJ=$(addsuffix .o, $(basename $(YANS_SRC)))
LIB_YANS=$(TOP)/libyans.so
YANS_PYTHON_OBJ=$(addsuffix .o, $(basename $(YANS_PYTHON_SRC)))
SIMULATOR_PYTHON_OBJ=$(addsuffix .o, $(basename $(SIMULATOR_PYTHON_SRC)))
LIB_SIMULATOR_PYTHON=$(TOP)/python/_simulatormodule.so

all: $(LIB_YANS) $(LIB_SIMULATOR_PYTHON)


ESCAPED_TOP_PYTHON_INSTALL=$(subst /,\/,$(TOP_PYTHON_INSTALL))
install: all $(INSTALL_DIRS)
	cp $(LIB_YANS) $(TOP_INSTALL);
	cp $(LIB_SIMULATOR_PYTHON) $(TOP_PYTHON_INSTALL)/simulator;
	for f in `find ./python -name __init__.py`; do \
		DEST=`echo $$f|sed -e 's/\.\/python/\.\/$(ESCAPED_TOP_PYTHON_INSTALL)/'`; \
		cp $$f $$DEST; \
	done

$(INSTALL_DIRS):
	mkdir -p $@;

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

$(LIB_YANS): $(YANS_OBJ)
	$(CXX) $(LDFLAGS) -shared -o $@ $^

$(YANS_PYTHON_OBJ): CXXFLAGS+=-I/usr/include/python2.3
$(LIB_SIMULATOR_PYTHON): $(SIMULATOR_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP) -lyans -shared -o $@ $^

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

TILDES=$(addsuffix ~, $(YANS_SRC))
TILDES+=$(addsuffix ~, $(YANS_PYTHON_SRC))

clean:
	rm -f $(YANS_OBJ) $(LIB_YANS) \
	$(YANS_PYTHON_OBJ) $(LIB_SIMULATOR_PYTHON) \
	$(TILDES) \
	2>/dev/null;
