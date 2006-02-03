all: build

include ./make/functions.mk
include ./make/platform.mk

PACKAGE_NAME=yans
PACKAGE_VERSION=0.6
PACKAGE_DIST= \
	make/platform.mk \
	make/rules.mk \
	make/functions.mk \
	make/yans.mk \
	make/samples.mk \
	make/python.mk \
	make/test.mk \
	Makefile \
	$(NULL)

TOP_BUILD_DIR=$(TOP)/bin
TOP_SRC_DIR=$(TOP)
DEFINES=-DRUN_SELF_TESTS=1
INCLUDES=\
 -I$(TOP_SRC_DIR)/simulator \
 -I$(TOP_SRC_DIR)/src/thread \
 -I$(TOP_SRC_DIR)/test \
 -I$(TOP_SRC_DIR)/src/common \
 -I$(TOP_SRC_DIR)/src/host \
 -I$(TOP_SRC_DIR)/src/arp \
 -I$(TOP_SRC_DIR)/src/ethernet \
 -I$(TOP_SRC_DIR)/src/ipv4 \
 -I$(TOP_SRC_DIR)/src/apps \
 $(NULL)
FLAGS=-Wall -Werror -O3 -gdwarf-2 -g3
LDFLAGS=
CXXFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS+=$(FLAGS) $(INCLUDES) $(DEFINES)
#VERBOSE=y


#TCP=bsd
PYTHON_PREFIX_INC=/usr/include/python2.3
PYTHON_PREFIX_LIB=/usr/lib
PYTHON_BIN=/usr/bin/python2.3
BOOST_PREFIX_LIB=/usr/lib
BOOST_PREFIX_INC=/usr/include




include make/yans.mk
include make/samples.mk
include make/python.mk
include make/test.mk

include make/rules.mk
