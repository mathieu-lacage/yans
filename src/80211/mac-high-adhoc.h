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
#ifndef MAC_HIGH_ADHOC_H
#define MAC_HIGH_ADHOC_H

#include "mac-address.h"

namespace yans {

class MacQueue80211e;
class Packet;
class Dcf;
class Packet;
class NetworkInterface80211;
class ChunkMac80211Hdr;

class MacHighAdhoc {
public:
	MacHighAdhoc ();
	virtual ~MacHighAdhoc ();

	void set_interface (NetworkInterface80211 *interface);
	void set_queue (MacQueue80211e *queue, Dcf *dcf);

	/* invoked by Mac80211. */
	void enqueue_from_ll (Packet *packet, MacAddress to);

	/* invoked by the MacLows. */
	void notify_ack_received_for (ChunkMac80211Hdr const *hdr);
	void receive (Packet *packet, ChunkMac80211Hdr const*hdr);
private:
	MacQueue80211e *m_queue;
	Dcf *m_dcf;
	NetworkInterface80211 *m_interface;
};

}; // namespace yans

#endif /* MAC_HIGH_ADHOC_H */
