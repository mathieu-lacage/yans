all: test-dlmopen libtestc.so test-printf.so 

CFLAGS:= -g -O0


SYSTEM_EXPORT_HEADERS:= system/src/stdio.h
SYSTEM_SRC := system/src/stdio.c
SYSTEM_OBJ := $(subst .c,.o,$(SYSTEM_SRC))
SYSTEM_INST_HEADERS:=$(subst /src/,/include/,$(SYSTEM_EXPORT_HEADERS))
SYSTEM_LIBC := system/lib/libc.so
system: $(SYSTEM_LIBC) $(SYSTEM_INST_HEADERS)
system/include/%.h: system/src/%.h
	cp $^ $@
system/src/%.o: CFLAGS += -fPIC
$(SYSTEM_LIBC): $(SYSTEM_OBJ)
	$(CC) -shared -o $@ $^

clean: 
	rm -f $(SYSTEM_INST_HEADERS) $(SYSTEM_OBJ) $(SYSTEM_LIBC) 2>/dev/null


#test-printf.so load-test test-dlmopen

test-dlmopen: LDFLAGS+=-ldl
test-dlmopen: test-dlmopen.o


libtestc.so: test-libc.o
	gcc -shared -nodefaultlibs -o $@ $^
test-libc.o: CFLAGS += -fPIC 
test-printf.so: test-printf.o
	gcc -shared -nodefaultlibs -L. -ltestc -o $@ $^
test-printf.o: CFLAGS += -fPIC

load-test: load-test.cc elf-module.cc reader.cc