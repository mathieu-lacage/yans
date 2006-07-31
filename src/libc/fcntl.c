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
#include "fcntl.h"
#include "mode-t.h"
#include "internal-libc.h"
#include "libc.h"
#include <stdarg.h>

int open(char const*pathname, int flags,...)
{
	if (flags & O_CREAT) {
		va_list arg;
		va_start(arg, flags);
		mode_t mode = va_arg(arg, int);
		va_end(arg);
		return g_libc->open_mode (g_libc, pathname, flags, mode);
	}
	return g_libc->open (g_libc, pathname, flags);
}
int creat(char const*pathname, mode_t mode)
{}
