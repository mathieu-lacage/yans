/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "stdio.h"
#include "internal-libc.h"
#include "libc.h"
#include <stdarg.h>


/* arbitrary values */
FILE *stdin;
FILE *stdout;
FILE *stderr;

FILE *fopen (char const *filename,
	     char const *modes)
{
	FILE *f = g_libc->fopen (g_libc, filename, modes);
	return f;
}
int fclose (FILE *fp)
{
	return g_libc->fclose (g_libc, fp);
}
size_t fread (void *ptr, size_t size,
	      size_t n, FILE *stream)
{
	return g_libc->fread (g_libc, ptr, size, n, stream);
}
size_t fwrite (void const* ptr, size_t size,
	       size_t n, FILE *stream)
{
	return g_libc->fwrite (g_libc, ptr, size, n, stream);
}


int printf(const char *format, ...)
{
	int retval;
	va_list ap;
	va_start (ap, format);
	retval = g_libc->vfprintf (g_libc, stdout, format, ap);
	va_end (ap);
	return retval;
}
int fprintf(FILE *stream, const char *format, ...)
{
	int retval;
	va_list ap;
	va_start (ap, format);
	retval = g_libc->vfprintf (g_libc, stream, format, ap);
	va_end (ap);
	return retval;
}	
int sprintf(char *str, const char *format, ...)
{
	int retval;
	va_list ap;
	va_start (ap, format);
	retval = g_libc->vsprintf (g_libc, str, format, ap);
	va_end (ap);
	return retval;
}	
int snprintf(char *str, size_t size, const char *format, ...)
{
	int retval;
	va_list ap;
	va_start (ap, format);
	retval = g_libc->vsnprintf (g_libc, str, size, format, ap);
	va_end (ap);
	return retval;
}	

int vprintf(const char *format, va_list ap)
{
	return g_libc->vfprintf (g_libc, stdout, format, ap);
}
int vfprintf(FILE *stream, const char *format, va_list ap)
{
	return g_libc->vfprintf (g_libc, stream, format, ap);
}
int vsprintf(char *str, const char *format, va_list ap)
{
	return g_libc->vsprintf (g_libc, str, format, ap);
}
int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return g_libc->vsnprintf (g_libc, str, size, format, ap);
}

