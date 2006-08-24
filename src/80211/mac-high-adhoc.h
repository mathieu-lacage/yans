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

#include "yans/mac-address.h"
#include "yans/callback.h"
#include "yans/packet.h"

namespace yans {

class DcaTxop;
class Packet;
class NetworkInterface80211;
class ChunkMac80211Hdr;

class MacHighAdhoc {
public:
	typedef Callback<void, Packet > ForwardCallback;

	MacHighAdhoc ();
	~MacHighAdhoc ();

	void set_interface (NetworkInterface80211 *interface);
	void set_forward_callback (ForwardCallback callback);
	void set_dca_txop (DcaTxop *dca);

	MacAddress get_bssid (void) const;

	void enqueue (Packet packet, MacAddress to);

	/* invoked by the MacLows. */
	void receive (Packet packet, ChunkMac80211Hdr const*hdr);
private:
	DcaTxop *m_dca;
	NetworkInterface80211 *m_interface;
	ForwardCallback m_callback;
};

}; // namespace yans

#endif /* MAC_HIGH_ADHOC_H */
