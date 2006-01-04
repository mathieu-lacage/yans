include ./functions.mk

TOP_INSTALL=$(TOP)/bin
TOP_PYTHON_INSTALL=$(TOP_INSTALL)/yans
DEFINES=-DRUN_SELF_TESTS=1
INCLUDES=-I$(TOP)/simulator -I$(TOP)/src/thread -I$(TOP)/test -I$(TOP)/src/common
FLAGS=-Wall -Werror -O0 -gdwarf-2
PYTHON_INCLUDES=-I/usr/include/python2.3

LDFLAGS=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
all: dirs build


# building of libyans.so
YANS_SRC= \
	simulator/clock.cc \
	simulator/event-heap.cc \
	simulator/event.cc \
	simulator/simulator.cc \
	src/thread/fiber-context-x86-linux-gcc.cc \
	src/thread/semaphore.cc \
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
	python/yans/__init__.py \
	python/yans/simulator/__init__.py \
	python/test-simulator.py \
	python/test-simulator-gc.py \
	$(NULL)
SIMULATOR_PYTHON_OBJ=$(call genobj, $(SIMULATOR_PYTHON_SRC))
LIB_SIMULATOR_PYTHON=$(TOP_INSTALL)/python/_simulatormodule.so
$(SIMULATOR_PYTHON_OBJ): CXXFLAGS+=$(PYTHON_INCLUDES)
$(LIB_SIMULATOR_PYTHON): $(SIMULATOR_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP_INSTALL) -lyans -shared -o $@ $(filter %.o,$^)
DIRS += $(call gendirs, $(SIMULATOR_PYTHON_SRC))
build: $(LIB_SIMULATOR_PYTHON)

# building of python models bindings
MODELS_PYTHON_SRC= \
	python/yans-models.cc \
	python/export-thread.cc \
	python/test-thread.py \
	$(NULL)
MODELS_PYTHON_OBJ=$(call genobj, $(MODELS_PYTHON_SRC))
LIB_MODELS_PYTHON=$(TOP_INSTALL)/python/_modelsmodule.so
$(MODELS_PYTHON_OBJ): CXXFLAGS+=$(PYTHON_INCLUDES)
$(LIB_MODELS_PYTHON): $(MODELS_PYTHON_OBJ)
	$(CXX) $(LDFLAGS) -lboost_python -L$(TOP_INSTALL) -lyans -shared -o $@ $(filter %.o,$^)
DIRS += $(call gendirs, $(MODELS_PYTHON_SRC))
build: $(LIB_MODELS_PYTHON)

# building of sample applications
SAMPLES_SRC= \
	src/samples/main-simulator.cc \
	src/samples/main-forwarding-simulator.cc \
	src/samples/main-forwarding-simulator-template.cc \
	$(NULL)
DIRS += $(call gendirs, $(SAMPLES_SRC))
SAMPLES=$(call genbin, $(basename $(SAMPLES_SRC)))
$(SAMPLES): %:%.o
	$(CXX) $(LDFLAGS) -L$(TOP_INSTALL) -lyans -o $@ $^
build: $(SAMPLES)



YANS_PYTHON_SRC= \
	$(SIMULATOR_PYTHON_SRC) \
	$(MODELS_PYTHON_SRC) \
	$(NULL)





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

depsclean:
	rm -f .deps 2>/dev/null

.deps:
	@echo "Generating dependency information.";
	@rm -f .deps 2>/dev/null; touch .deps;
	@-for f in `find ./ -name '*.c'`; do \
		$(CC) $(CFLAGS) -E -M -MP -MM -MT $(TOP_INSTALL)/$$f $$f 2>/dev/null >> .deps; \
	done;
	@-for f in `find ./ -name '*.cc'`; do \
		$(CXX) $(CXXFLAGS) -E -M -MP -MM -MT $(TOP_INSTALL)/$$f $$f 2>/dev/null >> .deps; \
	done;

-include .deps
