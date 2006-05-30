/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <stdint.h>
#include <cassert>
#include <iostream>

#define USE_CORBA 1

#include "yans/simulator.h"

using namespace yans;

#ifndef USE_CORBA
#include "local.h"
using namespace local;
#else
#include "local-corba.h"
using namespace localcorba;
#endif

int main (int argc, char *argv) 
{
	ComputingContext *ctx_a, *ctx_b;
	TestConnector *c;
	TestEventSource *a, *b;
#ifdef USE_CORBA
	ComputingContextFactory::start_servers ("desc.xml");
#endif
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
