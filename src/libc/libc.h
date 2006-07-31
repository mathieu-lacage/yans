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
#ifndef LIBC_H
#define LIBC_H

#include "size-t.h"
#include "ssize-t.h"
#include "mode-t.h"
#include <stdarg.h>

struct libc_FILE;
struct LibcPrivate;

struct Libc *libc_new (char const*root_path);
void libc_delete (struct Libc *libc);

/**
 * One might wonder why we use function pointers rather 
 * than straight functions. The idea is that if we used
 * functions, the caller would be need to be linked with these
 * functions. Here, instead, we can pass a pointer to an instance
 * of the Libc structure to A and make A call some of the function 
 * pointers defined in this structure without making A link against
 * an implementation of this code. The implementation is provided
 * at runtime.
 */
struct Libc {
	struct libc_FILE*(*fopen) (struct Libc *libc, char const *filename,char const *modes);
	int (*fclose) (struct Libc *libc, struct libc_FILE*fp);
	void (*clearerr) (struct Libc *libc, struct libc_FILE *stream);
	int (*feof) (struct Libc *libc, struct libc_FILE *stream);
	int (*ferror) (struct Libc *libc, struct libc_FILE *stream);
	int (*fileno) (struct Libc *libc, struct libc_FILE*stream);
	size_t (*fread) (struct Libc *libc, void *ptr, size_t size,size_t n, struct libc_FILE*stream);
	size_t (*fwrite) (struct Libc *libc, void const* ptr, size_t size,size_t n, struct libc_FILE*stream);
	int (*vfprintf) (struct Libc *libc, struct libc_FILE*stream, const char *format, va_list ap);
	int (*vsprintf) (struct Libc *libc, char *str, const char *format, va_list ap);
	int (*vsnprintf) (struct Libc *libc, char *str, size_t size, const char *format, va_list ap);

	int (*vopen) (struct Libc *libc, char const*pathname, int flags, va_list ap);
	ssize_t (*read) (struct Libc *libc, int fd, void *buf, size_t nbytes);
	ssize_t (*write) (struct Libc *libc, int fd, void const*buf, size_t n);
	int (*close) (struct Libc *libc, int fd);
	int (*socket) (struct Libc *libc, int domain, int type, int protocol);

	struct libc_FILE *stdout;
	struct libc_FILE *stderr;
	struct libc_FILE *stdin;
	struct LibcPrivate *priv;
};

#endif /* LIBC_H */
