#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[])
{
  void *retval;
  int i;
  for (i = 0; 1; i++) {
    retval = dlmopen (LM_ID_NEWLM, "./test-printf.so", RTLD_NOW);
    if (retval == 0) {
      printf ("error: %d -- %s\n", i, dlerror ());
      return 1;
    }
    retval = dlsym (retval, "main");
    printf ("main=0x%x\n", retval);
  }
  int b = 1;
  while (b) {}
  return 0;
}
