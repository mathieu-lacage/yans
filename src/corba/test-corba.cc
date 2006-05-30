/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <cassert>
#include <iostream>

#include "yans/simulator.h"
#include "local.h"

using namespace yans;
using namespace local;

int main (int argc, char *argv) 
{
	ComputingContext *ctx_a, *ctx_b;
	TestConnector *c;
	TestEventSource *a, *b;
	ctx_a = ComputingContextFactory::create ("a");
	ctx_b = ComputingContextFactory::create ("b");
	a = new TestEventSource (ctx_a);
	b = new TestEventSource (ctx_b);
	c = new TestConnector (20);
	a->connect (c);
	b->connect (c);

	a->start (5, 10000);
	b->start (11, 10000);

	Simulator::run ();

	std::cout << "a received " <<a->get_received () << std::endl;
	std::cout << "b received " <<b->get_received () << std::endl;

	delete a;
	delete b;
	delete c;
	
	return 0;
}
