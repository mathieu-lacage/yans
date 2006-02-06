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

#include "mac-80211.h"
#include "mac-high-access-point.h"
#include "mac-high-station.h"
#include "mac-high-qstation.h"
#include "mac-high-adhoc.h"
#include "phy-80211.h"
#include "hdr-mac-80211.h"
#include "mac-parameters.h"

#include <iostream>

static class Mac80211Class: public TclClass {
public:
        Mac80211Class() : TclClass("Mac/Mac80211") {}
        TclObject* create(int, const char*const*) {
                return (new Mac80211 ());
        }
} class_Mac80211;


Mac80211::Mac80211 ()
	: Mac ()
{}

Mac80211::~Mac80211 ()
{}

Phy80211 *
Mac80211::peekPhy80211 (void)
{
	// we peek into the parent class netif_ field.
	Phy80211 *phy = static_cast <class Phy80211 *> (netif_);
	return phy;
}

/* These three methods are the interface used by the higher-level
 * layers (i.e., arp, ll) to control where packets should be sent
 * by the MAC. The calling code theorically never accesses directly
 * the fields of the hdr_mac structure.
 * This interface is sooo badly broken, it is not funny:
 *  - uses default valued arguments
 *  - has different behavior depending on the value of the 
 *    default-valued argument (set vs get)
 *  - first argument is a char *hdr while it should be a Packet *
 *  - uses the magic values -2 and 0.
 *
 * *sigh*
 */
int
Mac80211::hdr_dst (char* hdr, int dst)
{
	class hdr_mac_80211 *mac_header = reinterpret_cast<class hdr_mac_80211 *>(hdr);
	if (dst != -2) {
		mac_header->setFinalDestination (dst);
	}
	return mac_header->getFinalDestination ();
}
int 
Mac80211::hdr_src (char* hdr, int src)
{
	class hdr_mac_80211 *mac_header = reinterpret_cast<class hdr_mac_80211 *>(hdr);
	if (src != -2) {
		mac_header->setSource (src);
	}
	return mac_header->getSource ();
}
int 
Mac80211::hdr_type(char *hdr, u_int16_t type)
{
	class hdr_mac_80211 *mac_header = reinterpret_cast<class hdr_mac_80211 *>(hdr);
	// XXX ? what is this data type sued for ?
	if (type != 0) {
		mac_header->setDataType (type);
	}
	return (int)mac_header->getDataType ();
}

/* This method is overriden to allow us to hook into
 * the construction process of the MAC object such that
 * we can complete construction of our own C++ objects.
 */
int 
Mac80211::command(int argc, const char*const* argv)
{
	if (argc == 3) {
		if (strcmp (argv[1], "mode") == 0) {
			if (strcmp (argv[2], "adhoc") == 0) {
				m_high = new MacHighAdhoc (this, peekPhy80211 ());
				return TCL_OK;
			} else if (strcmp (argv[2], "access-point") == 0) {
				m_high = new MacHighAccessPoint (this, peekPhy80211 ());
				return TCL_OK;
			} else if (strcmp (argv[2], "qaccess-point") == 0) {
				m_high = new MacHighAccessPoint (this, peekPhy80211 ());
				return TCL_OK;
			}
		}
	} else if (argc == 4) {
		if (strcmp (argv[1], "mode") == 0 &&
		    strcmp (argv[2], "station") == 0) {
			int ap = atoi (argv[3]);
			m_high = new MacHighStation (this, peekPhy80211 (), ap);
			return TCL_OK;
		}
		if (strcmp (argv[1], "mode") == 0 &&
		    strcmp (argv[2], "qstation") == 0) {
			int ap = atoi (argv[3]);
			m_high = new MacHighQStation (this, peekPhy80211 (), ap);
			return TCL_OK;
		}
	}
	int retval = Mac::command (argc, argv);
	return retval;
}

void 
Mac80211::recv (Packet *p, Handler *h)
{
	struct hdr_cmn *hdr = HDR_CMN(p);	
	
	switch(hdr->direction()) {
	case hdr_cmn::DOWN:
		m_high->enqueueFromLL (p);
		h->handle (p);
		break;
	case hdr_cmn::UP:
		m_high->receiveFromPhy (p);
		break;
	default:
		assert (FALSE);
		break;
	}
}

