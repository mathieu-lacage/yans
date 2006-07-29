#include <stdarg.h>

int printf(const char *format, ...)
{
  internal_printf ();
  return 0;
}
