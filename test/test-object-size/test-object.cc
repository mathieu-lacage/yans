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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#include "test-object.h"
#include "object-size.h"

#include <stdio.h>

#define noREALLY_TEST 1

void 
TestObject::run (Object *object)
{
#ifdef REALLY_TEST
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		object->set (0x66, i);
	}
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		if (object->get (i) != 0x66) {
			printf ("baad.\n");
		}
	}	
#endif
}

void 
TestObject::run (Object object)
{
#ifdef REALLY_TEST
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		object.set (0x67, i);
	}
	for (unsigned int i = 0; i < OBJECT_SIZE; i++) {
		if (object.get (i) != 0x67) {
			printf ("baad.\n");
		}
	}
#endif	
}
