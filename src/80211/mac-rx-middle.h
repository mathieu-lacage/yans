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

class MacLow;
class MacHigh;
class Packet;
class OriginatorRxStatus;

class MaxRxMiddle
{
public:
	MacRxMiddle (MacLow *low, MacHigh *high);

private:
	void gotPacket (double snr, int txMode);
	void gotData (Packet *packet);
	Packet *gotBlockAckReq (Packet *packet);

	std::map <int, OriginatorRxStatus *, std::less<int> > m_originatorStatus;
	std::map <pair<int,int>, OriginatorRxStatus *, std::less<pair<int,int> > > > m_qosOriginatorStatus;
};


#endif /* MAC_RX_MIDDLE_H */
