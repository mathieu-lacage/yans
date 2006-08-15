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

#ifndef MAC_RX_MIDDLE_H
#define MAC_RX_MIDDLE_H

#include <map>
#include <utility>
#include "callback.h"
#include "mac-address.h"
#include "gpacket.h"

namespace yans {

class ChunkMac80211Hdr;
class OriginatorRxStatus;

class MacRxMiddle
{
public:
	typedef Callback<void, Packet , ChunkMac80211Hdr const *> ForwardUpCallback;

	MacRxMiddle ();
	~MacRxMiddle ();

	void set_forward_callback (ForwardUpCallback callback);

	void receive (Packet packet, ChunkMac80211Hdr const *hdr);
private:
	OriginatorRxStatus *lookup (ChunkMac80211Hdr const*hdr);
	bool is_duplicate (ChunkMac80211Hdr const *hdr, OriginatorRxStatus *originator) const;
	Packet handle_fragments (Packet packet, ChunkMac80211Hdr const*hdr,
				  OriginatorRxStatus *originator, bool *complete);
	bool sequence_control_smaller (int seqa, int seqb);

	typedef std::map <MacAddress, OriginatorRxStatus *, std::less<MacAddress> > Originators;
	typedef std::map <std::pair<MacAddress, uint8_t>, OriginatorRxStatus *, std::less<std::pair<MacAddress,uint8_t> > > QosOriginators;
	typedef std::map <MacAddress, OriginatorRxStatus *, std::less<MacAddress> >::iterator OriginatorsI;
	typedef std::map <std::pair<MacAddress, uint8_t>, OriginatorRxStatus *, std::less<std::pair<MacAddress,uint8_t> > >::iterator QosOriginatorsI;
	Originators m_originator_status;
	QosOriginators m_qos_originator_status;
	ForwardUpCallback m_callback;
};

}; // namespace yans


#endif /* MAC_RX_MIDDLE_H */
