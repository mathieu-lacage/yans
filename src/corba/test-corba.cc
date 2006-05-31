/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <cassert>
#include <iostream>

#define noUSE_CORBA 1

#include "yans/simulator.h"

using namespace yans;

#ifndef USE_CORBA
#include "local.h"
using namespace local;
#else
#include "local-corba.h"
using namespace localcorba;
#endif

struct Topology {
	ComputingContext *ctx_a;
	ComputingContext *ctx_b;
	TestConnector *c;
	TestEventSource *a;
	TestEventSource *b;
};

static struct Topology
build_topology (void)
{
	struct Topology t;
	t.ctx_a = ComputingContextFactory::create ("a");
	t.ctx_b = ComputingContextFactory::create ("b");
	t.a = new TestEventSource (t.ctx_a);
	t.b = new TestEventSource (t.ctx_b);
	t.c = new TestConnector (20);
	t.a->connect (t.c);
	t.b->connect (t.c);

	t.a->start (5, 10000);
	t.b->start (11, 10000);

	return t;
}

static void
print_info (struct Topology topology)
{
	std::cout << "a received " <<topology.a->get_received () << std::endl;
	std::cout << "b received " <<topology.b->get_received () << std::endl;
}

static void
destroy (struct Topology t)
{
	delete t.a;
	delete t.b;
	delete t.c;

}



int main (int argc, char *argv) 
{
#ifdef USE_CORBA
	ComputingContextFactory::start_servers ("desc.xml");
#endif
	struct Topology t;
	t = build_topology ();

	Simulator::run ();

	print_info (t);
	destroy (t);
	return 0;
}
