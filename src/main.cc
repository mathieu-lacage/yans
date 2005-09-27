/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "test.h"

int main (int argc, char *argv[])
{
	TestManager *manager = new TestManager ();
	manager->enable_verbose ();
	manager->run_tests ();
	return 0;
}
