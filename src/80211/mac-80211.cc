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
#include "mac-low-80211.h"
#include "phy-80211.h"
#include "hdr-mac-80211.h"
#include "arf-mac-stations.h"

#include <iostream>

#define nopeTRACE_PACKETS 1


static class Mac80211Class: public TclClass {
public:
        Mac80211Class() : TclClass("Mac/Mac80211") {}
        TclObject* create(int, const char*const*) {
                return (new Mac80211 ());
        }
} class_Mac80211;


Mac80211::Mac80211 ()
	: Mac (),
	  m_stations (new ArfMacStations (this)),
	  m_low (new MacLow80211 (this))
{}

Mac80211::~Mac80211 ()
{}

class Phy80211 *
Mac80211::peekPhy80211 (void)
{
	// we peek into the parent class netif_ field.
	Phy80211 *phy = static_cast <class Phy80211 *> (netif_);
	return phy;
}

void
Mac80211::forwardUp (class Packet *packet)
{
#ifdef TRACE_PACKETS
	cout << "mac " << this << " rx " << HDR_CMN (packet)->size () << " from " << HDR_MAC_80211 (packet)->getSource () << endl;
#endif
	uptarget ()->recv (packet);
}

void
Mac80211::forwardDown (class Packet *packet)
{
	downtarget ()->recv (packet, (Handler *)0);
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
		mac_header->setDataType ((enum mac_80211_data_type)type);
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
	int retval;
	retval = Mac::command (argc, argv);
	if (argc == 3 &&
	    strcmp(argv[1], "netif") == 0) {
		m_low->completeConstruction (peekPhy80211 ());
	}
	if (argc >= 3) {
		if (strcmp (argv[1], "mode") == 0) {
			if (strcmp (argv[2], "adhoc") == 0) {
				//m_high = 0;
			}
			if (argc == 3) {
				// bad
			} else if (strcmp (argv[2], "station-passive") == 0) {
				//m_high = new MacHighStation (argv[3]);
			} else if (strcmp (argv[2], "station-active") == 0) {
				//m_high = new MacHighStation (argv[3]);
			} else if (strcmp (argv[2], "access-point") == 0) {
				//m_high = new MacHighAccessPoint (argv[3]);
			}
		}
	}
	return retval;
}

void 
Mac80211::recv (Packet *p, Handler *h)
{
	struct hdr_cmn *hdr = HDR_CMN(p);	
	
	switch(hdr->direction()) {
	case hdr_cmn::DOWN:
#ifdef TRACE_PACKETS
		cout << "mac " << this << " queue tx " << HDR_CMN (p)->size () << " for " << HDR_MAC_80211 (p)->getDestination () << endl;
#endif
		m_low->enqueue (p);
		h->handle (p);
		break;
	case hdr_cmn::UP:
		m_low->receive (p);
		break;
	default:
		assert (FALSE);
		break;
	}
}

MacStation *
Mac80211::lookupStation (int address)
{
	return m_stations->lookup (address);
}
