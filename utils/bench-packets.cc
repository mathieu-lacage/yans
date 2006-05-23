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
#include "yans/wall-clock-ms.h"
#include "yans/packet.h"
#include "yans/chunk-constant-data.h"
#include "yans/chunk-udp.h"
#include "yans/chunk-ipv4.h"
#include "yans/chunk-mac-80211-hdr.h"
#include <iostream>

using namespace yans;

static void 
create_packets (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;
	ChunkMac80211Hdr hdr;

	for (uint32_t i = 0; i < n; i++) {
		Packet *p = PacketFactory::create ();
		p->add (&data);
		p->add (&udp);
		p->add (&ipv4);
		p->add (&hdr);
		p->unref ();
	}
}

int main (int argc, char *argv[])
{
	WallClockMs time;
	uint32_t n = 0;
	while (argc > 0) {
		if (strncmp ("--n=", argv[0],strlen ("--n=")) == 0) {
			char const *n_ascii = argv[0] + strlen ("--n=");
			n = atoi (n_ascii);
		}
		argc--;
		argv++;
	}
	time.start ();
	create_packets (n);
	unsigned long long delta_ms = time.end ();
	double ps = n;
	ps *= 1000;
	ps /= delta_ms;
	std::cout << ps << " packets/s" << std::endl;
	return 0;
}
