include ./functions.mk

TOP_INSTALL=$(TOP)/bin
TOP_PYTHON_INSTALL=$(TOP_INSTALL)/yans
DEFINES=-DRUN_SELF_TESTS=1
INCLUDES=-I$(TOP)/simulator -I$(TOP)/src/thread -I$(TOP)/test
FLAGS=-Wall -Werror -O0 -gdwarf-2

LDFLAGS=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
all: dirs build


# building of libyans.so
YANS_SRC= \
	simulator/clock.cc \
	simulator/event-heap.cc \
	simulator/event.cc \
	simulator/simulator-simple.cc \
	src/thread/fiber-context-x86-linux-gcc.cc \
	src/thread/semaphore.cc \
	src/thread/runnable.cc \
	src/thread/fiber.cc \
	src/thread/fiber-scheduler.cc \
	src/thread/thread.cc \
	test/test.cc \
	$(NULL)
YANS_OBJ=$(call genobj, $(YANS_SRC))
LIB_YANS=$(TOP_INSTALL)/libyans.so
$(YANS_OBJ): CXXFLAGS += -fPIC
$(YANS_OBJ): CFLAGS += -fPIC
$(LIB_YANS): $(YANS_OBJ)
	$(CXX) $(LDFLAGS) -shared -o $@ $(filter %.o,$^)
DIRS += $(call gendirs, $(YANS_SRC))
build: $(LIB_YANS)


# building of main-test
MAIN_TEST_SRC=test/main-test.cc
MAIN_TEST_OBJ=$(call genobj, $(MAIN_TEST_SRC))
MAIN_TEST=$(call genbin, test/main-test)
$(MAIN_TEST): $(MAIN_TEST_OBJ)
	$(CXX) $(CXXFLAGS) -lyans -L$(TOP_INSTALL) -o $@ $^
DIRS += $(call gendirs, $(YANS_SRC))
build: $(MAIN_TEST)


# building of python bindings.
SIMULATOR_PYTHON_SRC= \
	python/yans-simulator.cc \
	python/export-simulator.cc \
	python/export-event.cc \
	python/__init__.py \
	python/simulator/__init__.py \
	$(NULL)
YANS_PYTHON_SRC= \
	$(SIMULATOR_PYTHON_SRC) \
	$(NULL)
SIMULATOR_PYTHON_OBJ=$(call genobj, $(YANS_PYTHON_SRC))
LIB_SIMULATOR_PYTHON=$(TOP_INSTALL)/python/_simulatormodule.so
$(SIMULATOR_PYTHON_OBJ): CXXFLAGS+=-I/usr/include/python2.3
$(LIB_SIMULATOR_PYTHON): $(SIMULATOR_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP_INSTALL) -lyans -shared -o $@ $(filter *.o,$^)
DIRS += $(call gendirs, $(SIMULATOR_PYTHON_SRC))
build: $(LIB_SIMULATOR_PYTHON)



TMP_DIRS=$(sort $(DIRS))
dirs: $(TMP_DIRS)
$(TMP_DIRS):
	mkdir -p $@;

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@$(MAKE) -C $@


$(TOP_INSTALL)/%.py:%.py
	cp $^ $@
$(TOP_INSTALL)/%.o:%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^
$(TOP_INSTALL)/%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	find ./ -name '*~'|xargs rm -f 2>/dev/null;
	rm -rf $(TOP_INSTALL) 2>/dev/null;
