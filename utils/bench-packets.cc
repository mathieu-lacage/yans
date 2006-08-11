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
#include "yans/gpacket.h"
#include "yans/chunk-constant-data.h"
#include "yans/chunk-udp.h"
#include "yans/chunk-ipv4.h"
#include <iostream>

using namespace yans;

static void 
bench_ptr_a (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;

	for (uint32_t i = 0; i < n; i++) {
		PacketPtr p = Packet::create ();
		p->add (&data);
		p->add (&udp);
		p->add (&ipv4);
		PacketPtr o = p->copy ();
		o->peek (&ipv4);
		o->remove (&ipv4);
		o->peek (&udp);
		o->remove (&udp);
		o->peek (&data);
		o->remove (&data);
	}
}

static void 
bench_ptr_b (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;

	for (uint32_t i = 0; i < n; i++) {
		PacketPtr p = Packet::create ();
		p->add (&data);
		p->add (&udp);
		p->add (&ipv4);
	}
}

static void
ptr_c2 (PacketPtr p)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;

	p->peek (&udp);
	p->remove (&udp);
	p->peek (&data);
	p->remove (&data);
}

static void 
ptr_c1 (PacketPtr p)
{
	ChunkIpv4 ipv4;
	p->peek (&ipv4);
	p->remove (&ipv4);
	ptr_c2 (p);
}

static void
bench_ptr_c (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;

	for (uint32_t i = 0; i < n; i++) {
		PacketPtr p = Packet::create ();
		p->add (&data);
		p->add (&udp);
		p->add (&ipv4);
		ptr_c1 (p);
	}
}

static void 
bench_cow_a (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;

	for (uint32_t i = 0; i < n; i++) {
		GPacket p;
		p.add (&data);
		p.add (&udp);
		p.add (&ipv4);
		GPacket o = p;
		o.peek (&ipv4);
		o.remove (&ipv4);
		o.peek (&udp);
		o.remove (&udp);
		o.peek (&data);
		o.remove (&data);
	}
}

static void 
bench_cow_b (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;

	for (uint32_t i = 0; i < n; i++) {
		GPacket p;
		p.add (&data);
		p.add (&udp);
		p.add (&ipv4);
	}
}

static void
cow_c2 (GPacket p)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;

	p.peek (&udp);
	p.remove (&udp);
	p.peek (&data);
	p.remove (&data);
}
static void
cow_c1 (GPacket p)
{
	ChunkIpv4 ipv4;

	p.peek (&ipv4);
	p.remove (&ipv4);

	cow_c2 (p);
}

static void 
bench_cow_c (uint32_t n)
{
	ChunkConstantData data = ChunkConstantData (2000, 1);
	ChunkUdp udp;
	ChunkIpv4 ipv4;

	for (uint32_t i = 0; i < n; i++) {
		GPacket p;
		p.add (&data);
		p.add (&udp);
		p.add (&ipv4);
		cow_c1 (p);
	}
}

static void
run_bench (void (*bench) (uint32_t), uint32_t n, char const *name)
{
	WallClockMs time;
	time.start ();
	(*bench) (n);
	unsigned long long delta_ms = time.end ();
	double ps = n;
	ps *= 1000;
	ps /= delta_ms;
	std::cout << name<<"=" << ps << " packets/s" << std::endl;
}

int main (int argc, char *argv[])
{
	uint32_t n = 0;
	bool ptr = true;
	while (argc > 0) {
		if (strncmp ("--n=", argv[0],strlen ("--n=")) == 0) {
			char const *n_ascii = argv[0] + strlen ("--n=");
			n = atoi (n_ascii);
		}
		if (strncmp ("--packet=", argv[0], strlen ("--packet=")) == 0) {
			char const *packet_type = argv[0] + strlen ("--packet=");
			if (strcmp ("ptr", packet_type) == 0) {
				ptr = true;
			} else if (strcmp ("cow", packet_type) == 0) {
				ptr = false;
			}
		}
		argc--;
		argv++;
	}

	if (ptr) {
		run_bench (&bench_ptr_a, n, "a");
	} else {
		run_bench (&bench_cow_a, n, "a");
	}
	if (ptr) {
		run_bench (&bench_ptr_b, n, "b");
	} else {
		run_bench (&bench_cow_b, n, "b");
	}
	if (ptr) {
		run_bench (&bench_ptr_c, n, "c");
	} else {
		run_bench (&bench_cow_c, n, "c");
	}

	return 0;
}
