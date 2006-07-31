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
#include "libc.h"
#include <string>
#include <stdio.h>
#include <sys/fcntl.h>

struct LibcPrivate {
	std::string root_path;
};

struct libc_FILE {
	FILE file;
};


struct libc_FILE*
my_fopen (struct Libc *libc, char const *filename,char const *modes)
{
	std::string real_filename = libc->priv->root_path;
	real_filename.append ("/");
	real_filename.append (filename);
	FILE *file = ::fopen (real_filename.c_str (), modes);
	return (libc_FILE*)file;
}
int 
my_fclose (struct Libc *libc, struct libc_FILE*fp)
{
	return ::fclose ((FILE*)fp);
}
size_t 
my_fread (struct Libc *libc, void *ptr, size_t size,size_t n, struct libc_FILE*stream)
{
	return ::fread (ptr, size, n, (FILE*)stream);
}
size_t 
my_fwrite (struct Libc *libc, void const* ptr, size_t size,size_t n, struct libc_FILE*stream)
{
	return ::fwrite (ptr, size, n, (FILE*)stream);
}

int 
my_vfprintf (struct Libc *libc, struct libc_FILE*stream, const char *format, va_list ap)
{
	return ::vfprintf ((FILE*)stream, format, ap);
}
int 
my_vsprintf (struct Libc *libc, char *str, const char *format, va_list ap)
{
	return ::vsprintf (str, format, ap);
}
int 
my_vsnprintf (struct Libc *libc, char *str, size_t size, const char *format, va_list ap)
{
	return ::vsnprintf (str, size, format, ap);
}

int 
my_open (struct Libc *libc, char const*pathname, int flags)
{
	std::string real_filename = libc->priv->root_path;
	real_filename.append ("/");
	real_filename.append (pathname);
	return ::open (real_filename.c_str (), flags);
}
int 
my_open_mode (struct Libc *libc, char const*pathname, int flags, mode_t mode)
{
	std::string real_filename = libc->priv->root_path;
	real_filename.append ("/");
	real_filename.append (pathname);
	return ::open (real_filename.c_str (), flags, mode);
}
ssize_t 
my_read (struct Libc *libc, int fd, void *buf, size_t nbytes)
{
	return ::read (fd, buf, nbytes);
}
ssize_t 
my_write (struct Libc *libc, int fd, void const*buf, size_t n)
{
	return ::write (fd, buf, n);
}
int 
my_close (struct Libc *libc, int fd)
{
	return ::close (fd);
}
int 
my_socket (struct Libc *libc, int domain, int type, int protocol)
{
	return 0;
}


struct Libc *
libc_new (char const*root_path)
{
	struct Libc *libc = new struct Libc ();
	libc->fopen = my_fopen;
	libc->fclose = my_fclose;
	libc->fread = my_fread;
	libc->fwrite = my_fwrite;
	libc->vfprintf = my_vfprintf;
	libc->vsprintf = my_vsprintf;
	libc->vsnprintf = my_vsnprintf;
	libc->open = my_open;
	libc->open_mode = my_open_mode;
	libc->read = my_read;
	libc->write = my_write;
	libc->close = my_close;
	libc->socket = my_socket;
	libc->priv = new struct LibcPrivate ();

	libc->priv->root_path = root_path;
	std::string filename = root_path;
	filename.append ("/stdin");
	libc->stdin = (libc_FILE*)::fopen (filename.c_str (), "r");
	filename = root_path;
	filename.append ("/stdout");
	libc->stdout = (libc_FILE*)::fopen (filename.c_str (), "r");
	filename = root_path;
	filename.append ("/stderr");
	libc->stdout = (libc_FILE*)::fopen (filename.c_str (), "r");
	return libc;
}
void 
libc_delete (struct Libc *libc)
{
	delete libc->priv;
	delete libc;
}
