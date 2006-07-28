all: test-printf.so load-test

test-printf.so: test-printf.o
	gcc -shared -o $@ $^

test-printf.o: test-printf.c
	gcc -fPIC $(CFLAGS) -c -o $@ $^

load-test: load-test.cc elf-module.cc reader.cc