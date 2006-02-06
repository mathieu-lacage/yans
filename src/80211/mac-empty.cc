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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "mac-empty.h"

#include <iostream>


static class MacEmptyClass: public TclClass {
public:
        MacEmptyClass() : TclClass("Mac/MacEmpty") {}
        TclObject* create(int, const char*const*) {
                return (new MacEmpty ());
        }
} class_MacEmpty;


MacEmpty::MacEmpty ()
	: Mac ()
{}

MacEmpty::~MacEmpty ()
{}


void 
MacEmpty::recv (Packet *p, Handler *h)
{
	struct hdr_cmn *hdr = HDR_CMN(p);	

	switch(hdr->direction()) {
	case hdr_cmn::DOWN:
		cout << "mac down" << endl;
		downtarget ()->recv (p);
		break;
	case hdr_cmn::UP :
		cout << "mac up" << endl;
		uptarget ()->recv (p);
		break;
	default:
		assert (FALSE);
		break;
	}
}
