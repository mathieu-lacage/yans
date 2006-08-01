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
#include "unistd.h"
#include "internal-libc.h"
#include "indirect-libc.h"

int close (int fd)
{
	return g_libc->close (g_libc, fd);
}
ssize_t read (int fd, void *buf, size_t nbytes)
{
	return g_libc->read (g_libc, fd, buf, nbytes);
}
ssize_t write (int fd, void const*buf, size_t n)
{
	return g_libc->write (g_libc, fd, buf, n);
}
