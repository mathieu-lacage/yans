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
#include <string>
#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "libc.h"


#define NOBIT_OPEN_FILES 8
#define NO_OPEN_FILES (NOBIT_OPEN_FILES<<8)

struct fd {
	bool is_alloc;
	bool is_socket;
	union {
		class FileFd *file;
		class SocketFd *socket;
	} u;
};

struct LibcPrivate {
	std::string root_path;
	struct fd fds[NO_OPEN_FILES];
};

struct libc_FILE {
	FILE file;
};

class FileFd {
 public:
	FileFd (int real_file);
	int get_fd (void);
 private:
	int m_fd;
};

class SocketFd {
};

FileFd::FileFd (int real_file)
	: m_fd (real_file)
{}
int 
FileFd::get_fd (void)
{
	return m_fd;
}

static void
set_simulation_errno (struct Libc *libc, int no)
{
	// XXX ?
}

static int
alloc_fd (struct LibcPrivate *priv)
{
	for (int i = 0; i < NO_OPEN_FILES; i++) {
		if (!priv->fds[i].is_alloc) {
			priv->fds[i].is_alloc = true;
			return i;
		}
	}
	return -1;
}


struct libc_FILE*
my_fopen (struct Libc *libc, char const *filename,char const *modes)
{
	std::string real_filename = libc->priv->root_path;
	real_filename.append ("/");
	real_filename.append (filename);
	errno = 0;
	FILE *file = ::fopen (real_filename.c_str (), modes);
	if (file == 0) {
		set_simulation_errno (libc, errno);
		return 0;
	}
	// finally, we record a fd associated to this stream.
	int fd = alloc_fd (libc->priv);
	if (fd == -1) {
		set_simulation_errno (libc, ENFILE);
		return 0;
	}
	int real_fd = ::fileno (file);
	libc->priv->fds[fd].is_socket = false;
	libc->priv->fds[fd].u.file = new FileFd (real_fd);
	return (libc_FILE*)file;
}
void my_clearerr (struct Libc *libc, struct libc_FILE *stream)
{
	// does not set errno.
	::clearerr ((FILE*)stream);
}
int my_feof (struct Libc *libc, struct libc_FILE *stream)
{
	// does not set errno.
	return ::feof ((FILE*)stream);
}
int my_ferror (struct Libc *libc, struct libc_FILE *stream)
{
	// does not set errno.
	return ::ferror ((FILE*)stream);
}

int
my_fileno (struct Libc *libc, struct libc_FILE *stream)
{
	int real_fd = ::fileno ((FILE*)stream);
	if (real_fd == -1) {
		set_simulation_errno (libc, EBADF);
		return -1;
	}
	for (int i = 0; i < NO_OPEN_FILES; i++) {
		if (libc->priv->fds[i].is_alloc &&
		    !libc->priv->fds[i].is_socket &&
		    libc->priv->fds[i].u.file->get_fd () == real_fd) {
			return i;
		}
	}
	set_simulation_errno (libc, EBADF);
	return -1;
}
int 
my_fclose (struct Libc *libc, struct libc_FILE*fp)
{
	int fd = my_fileno (libc, fp);
	if (fd == -1) {
		set_simulation_errno (libc, EBADF);
		return -1;
	}
	libc->priv->fds[fd].is_alloc = 0;
	delete libc->priv->fds[fd].u.file;
	errno = 0;
	int retval = ::fclose ((FILE*)fp);
	if (retval == 1) {
		set_simulation_errno (libc, retval);
		return -1;
	}
	return retval;
}
size_t 
my_fread (struct Libc *libc, void *ptr, size_t size,size_t n, struct libc_FILE*stream)
{
	size_t retval = ::fread (ptr, size, n, (FILE*)stream);
	return retval;
	
}
size_t 
my_fwrite (struct Libc *libc, void const* ptr, size_t size,size_t n, struct libc_FILE*stream)
{
	size_t retval = ::fwrite (ptr, size, n, (FILE*)stream);
	return retval;
}

int 
my_vfprintf (struct Libc *libc, struct libc_FILE*stream, const char *format, va_list ap)
{
	// does not set errno.
	return ::vfprintf ((FILE*)stream, format, ap);
}
int 
my_vsprintf (struct Libc *libc, char *str, const char *format, va_list ap)
{
	// does not set errno.
	return ::vsprintf (str, format, ap);
}
int 
my_vsnprintf (struct Libc *libc, char *str, size_t size, const char *format, va_list ap)
{
	// does not set errno.
	return ::vsnprintf (str, size, format, ap);
}

int 
my_vopen (struct Libc *libc, char const*pathname, int flags, va_list ap)
{
	std::string real_filename = libc->priv->root_path;
	real_filename.append ("/");
	real_filename.append (pathname);
	int retval;
	errno = 0;
	if (flags & O_CREAT) {
		int mode = va_arg (ap, int);
		retval = ::open (real_filename.c_str (), flags, mode);
	} else {
		retval = ::open (real_filename.c_str (), flags);
	}
	if (retval == -1) {
		set_simulation_errno (libc, errno);
		return -1;
	}
	int simu_fd = alloc_fd (libc->priv);
	libc->priv->fds[simu_fd].is_socket = false;
	libc->priv->fds[simu_fd].u.file = new FileFd (retval);	
	return simu_fd;
}
ssize_t 
my_read (struct Libc *libc, int fd, void *buf, size_t n)
{
	errno = 0;
	int retval = ::read (libc->priv->fds[fd].u.file->get_fd (), buf, n);
	if (retval == -1) {
		set_simulation_errno (libc, errno);
		return -1;
	}
	return retval;
}
ssize_t 
my_write (struct Libc *libc, int fd, void const*buf, size_t n)
{
	errno = 0;
	int retval = ::write (libc->priv->fds[fd].u.file->get_fd (), buf, n);
	if (retval == -1) {
		set_simulation_errno (libc, errno);
		return -1;
	}
	return retval;
}
int 
my_close (struct Libc *libc, int fd)
{
	// does not set errno ? XXX
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
	libc->vopen = my_vopen;
	libc->read = my_read;
	libc->write = my_write;
	libc->close = my_close;
	libc->socket = my_socket;
	libc->priv = new struct LibcPrivate ();

	for (int i = 0; i < NO_OPEN_FILES; i++) {
		libc->priv->fds[i].is_alloc = false;
	}
	libc->stdin = my_fopen (libc, "stdin", "r");
	libc->stdout = my_fopen (libc, "stdout", "w");
	libc->stderr = my_fopen (libc, "stderr", "w");
	return libc;
}
void 
libc_delete (struct Libc *libc)
{
	delete libc->priv;
	delete libc;
}
