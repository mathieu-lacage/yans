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


class Packet;
class OriginatorRxStatus;
class NetInterface80211;

class MacRxMiddle
{
public:
	MacRxMiddle ();

	void set_interface (NetInterface80211 *interface);

	void send_up (Packet *packet);
private:
	OriginatorRxStatus *lookupQos (int source, int TID);
	OriginatorRxStatus *lookupNqos (int source);
	OriginatorRxStatus *lookup (Packet *packet);
	bool handle_duplicates (Packet *packet, OriginatorRxStatus *originator);
	bool handle_fragments (Packet *packet, OriginatorRxStatus *originator);
	void drop_packet (Packet *packet);
	bool sequence_control_smaller (int seqa, int seqb);

	std::map <int, OriginatorRxStatus *, std::less<int> > m_originatorStatus;
	std::map <std::pair<int,int>, OriginatorRxStatus *, std::less<std::pair<int,int> > > m_qosOriginatorStatus;
	NetInterface80211 *m_interface;
};


#endif /* MAC_RX_MIDDLE_H */
