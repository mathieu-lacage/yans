# building of main-test
TEST_SRC=test/main-test.cc
TEST_OUTPUT=$(call gen-bin, test/main-test)
TEST_CXXFLAGS=$(CXXFLAGS)
TEST_LDFLAGS=$(LDFLAGS) -lyans -L$(TOP_BUILD_DIR)

ALL += \
	TEST \
	$(NULL)
