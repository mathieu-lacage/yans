#include <stdarg.h>
int (*internal_vprintf) (const char *format, va_list ap);

int set_internal_vprintf (int (*fn) (const char *,va_list))
{
  internal_vprintf = fn;
}

int printf (const char *format,...)
{
  va_list ap;
  va_start (ap, format);
  internal_vprintf (format, ap);
  va_end (ap);
}
