/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
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
#ifndef STDIO_H
#define STDIO_H

#include "bits/size-t.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct libc_FILE FILE;


extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

FILE *fopen (char const *filename,
	     char const *modes);
int fclose (FILE *fp);
size_t fread (void *ptr, size_t size,
	      size_t n, FILE *stream);
size_t fwrite (void const* ptr, size_t size,
	       size_t n, FILE *stream);


int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

#include <stdarg.h>

int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#ifdef __cplusplus
};
#endif

#endif /* STDIO_H */
