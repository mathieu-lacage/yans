/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "test-object.h"
#include "object-size.h"

#include <stdio.h>

void 
TestObject::run (Object *object)
{
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		object->set (0x66, i);
	}
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		if (object->get (i) != 0x66) {
			printf ("baad.\n");
		}
	}	
}

void 
TestObject::run (Object object)
{
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		object.set (0x67, i);
	}
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		if (object.get (i) != 0x67) {
			printf ("baad.\n");
		}
	}	
}
