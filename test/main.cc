/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "object.h"
#include "time.h"
#include "test-object.h"
#include <stdio.h>

int main (int argc, char *arg[])
{
	TestObject test;
	Time time;
	time.start ();
	for (int i = 0; i < 100000; i++) {
		Object *object = new Object ();
		test.run (object);
	}
	printf ("indirect: %llums", time.end ());
	time.start ();
	for (int i = 0; i < 100000; i++) {
		Object object = Object ();
		test.run (object);
	}
	printf ("direct: %llums", time.end ());

	return 0;
}
