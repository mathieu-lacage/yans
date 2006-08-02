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
#include <stdio.h>
#include <pthread.h>

static __thread int g_var;

static void *run_a (void *data)
{
	g_var = (int)data;
}

static void *run_b (void *data)
{
	g_var = (int)data;
}

int main (int argc, char *argv[])
{
	printf ("starting process %s\n" , argv[0]);

	pthread_t th;
	pthread_attr_t attr;
	int retval;

	retval = pthread_create (&th, 0, run_a, (void*)2);
	if (retval != 0) {
		goto err;
	}
	retval = pthread_create (&th, 0, run_a, (void*)3);
	if (retval != 0) {
		goto err;
	}

	return 0;
 err:
	return 1;
}
