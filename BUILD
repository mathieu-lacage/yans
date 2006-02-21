The GNU Make Makefile written for yans is a bit complicated which means
that reading it to figure out what targets to invoke and what variables
to define to use it can be a bit tricky. Here is the current list of things
you can use.

1) Variables
------------

- TOP_BUILD_DIR:
  all the files generated during the build of the project are
  stored there. This variable is set to "bin" by default.
  example: make TOP_BUILD_DIR=/tmp/build-dir

- OPTI_FLAGS:
  the flags to pass to the compiler for optimization purposes.
  This variable is set to -O0 by default. This variable is
  not very useful since you can use the "opti" target.
  example: make OPTI_FLAGS=-O3

- VERBOSE:
  if set to a non-empty value, ("y" for example), each compilation
  command is output on the terminal. Otherwise, short build messages
  are output on the terminal. This variable is not set by default
  which means that only the short build messages are output.
  example: make VERBOSE=y

2) Targets
----------

- all: 
  build a non-optimized debug build of the code located in TOP_BUILD_DIR.
  asserts are enabled.

- clean:
  remove the TOP_BUILD_DIR directory

- cleano:
  remove all object files from TOP_BUILD_DIR

- opti:
  build an optimized version of the code located in TOP_BUILD_DIR/opti.
  asserts are disabled.

- opti-arc:
  build an optimized version of the code with arc profiling information,
  run the programs specified by the run-opti-arc-profile-hook target,
  rebuild an optimized version of the code for this specific load.
  The code is built in TOP_BUILD_DIR/opti-arc.

- gcov:
  build a non-optimized version of the code with arc profiling information
  and coverage analysis code. You can then run various programs to generate
  the coverage information and then use gcov itself to gather user output
  from this coverage information.
  The code is built in TOP_BUILD_DIR/gcov

- dist:
  build a tarball of the project named PACKAGE_NAME-PACKAGE_VERSION.tar.gz
  which decompresses in PACKAGE_NAME-PACKAGE_VERSION by default and contains
  all the code we ship.

- distcheck:
  the same as target dist but also decompresses the generated tarball, and
  attempts to build its default target.

- fastdist:
  the same as dist except that we do not delete the temporary directory 
  used to generate the tarball once it has been generated.

- fastdistcheck:
  the same as distcheck but we do not delete the temporary directory
  used to build the decompressed tarball before running distcheck.


