/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "test.h"

#include <stdio.h>
static void
write_buffer (char const *filename, uint8_t *data, uint32_t size)
{
	FILE *file = fopen (filename, "w");
	uint32_t written = 0;
	while (written < size) {
		size_t tmp = fwrite (data, 1, size - written, file); 
		written += tmp;
		data += tmp;
	}
}

#include "buffer.h"
#include "packet.h"
#include "chunk-tcp.h"
#include "chunk-ipv4.h"
static void 
run_one_test (void)
{
	Packet *packet = new Packet ();
	Buffer *buffer = new Buffer ();
	ChunkIpv4 *ipv4 = new ChunkIpv4 ();
	ChunkTcp *tcp = new ChunkTcp ();
	packet->add_header (tcp);
	packet->add_header (ipv4);

	packet->serialize (buffer);

	packet->unref ();

	write_buffer ("mathieu", buffer->peek_data (), buffer->get_current ());
}

int main (int argc, char *argv[])
{
#ifdef RUN_SELF_TESTS
	TestManager *manager = new TestManager ();
	manager->enable_verbose ();
	manager->run_tests ();
#endif /* RUN_SELF_TESTS */

	run_one_test ();

	return 0;
}
