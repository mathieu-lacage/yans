#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[])
{
  void *module, *symbol;
  int i;
  for (i = 0; 1; i++) {
    module = dlmopen (LM_ID_NEWLM, "./test-printf.so", RTLD_NOW);
    if (module == 0) {
      printf ("error: %d -- %s\n", i, dlerror ());
      break;
    }
    symbol = dlsym (module, "set_internal_vprintf");
    if (symbol == 0) {
      printf ("error looking up set_internal_vprintf\n");
      break;
    }
    int (*set_internal_vprintf) (int (*) (const char *,va_list)) = symbol;
    printf ("set_internal_vprintf\n");
    set_internal_vprintf (&vprintf);
    printf ("set_internal_vprintf done\n");

    symbol = dlsym (module, "main");
    if (symbol == 0) {
      printf ("error looking up main\n");
      break;
    }
    int (*local_main) (int, char*[]) = symbol;
    printf ("local main\n");
    local_main (argc, argv);
    printf ("local main done\n", symbol);
  }
  int b = 1;
  while (b) {}
  return 0;
}
