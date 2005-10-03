/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "object.h"
#include "time.h"
#include "test-object.h"
#include <stdio.h>

int main (int argc, char *arg[])
{
	TestObject test;
	printf ("sizeof: %lu\n", sizeof (Object));
	Time time;
	time.start ();
	for (int i = 0; i < 100000; i++) {
		Object *object = new Object ();
		for (int j = 0; j < 100; j++) {
			test.run (object);
		}
	}
	printf ("indirect: %llums\n", time.end ());
	time.start ();
	for (int i = 0; i < 100000; i++) {
		Object object = Object ();
		for (int j = 0; j < 100; j++) {
			test.run (object);
		}
	}
	printf ("direct: %llums\n", time.end ());

	return 0;
}
