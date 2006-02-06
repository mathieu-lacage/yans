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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef MAC_QUEUE_80211E
#define MAC_QUEUE_80211E

/* This queue implements what is needed for the 802.11e standard
 * Specifically, it refers to 802.11e/D9, section 9.9.1.6, paragraph 6.
 *
 * When a packet is received by the MAC, to be sent to the PHY, 
 * it is queued in the internal queue after being tagged by the 
 * current time. If the queue is non-empty (quite likely), we
 * notify m_packetNotify. This information is forwarded to 
 * The associated MacLow80211 which might try to dequeue packets
 * from this queue if it is not doing anything else more important.
 *
 * If it is doing something too important to handle new packets,
 * the MacLow80211 is supposed to try to dequeue packets the next 
 * time it can.
 *
 * When a packet is dequeued, the queue checks its timestamp 
 * to verify whether or not it should be dropped. If 
 * dot11EDCATableMSDULifetime has elapsed, it is dropped.
 * Otherwise, it is returned to the caller.
 */

#include <list>
#include <utility>

class MacParameters;
class Packet;

class MacQueue80211e {
public:
	MacQueue80211e ();
	~MacQueue80211e ();

	void setParameters (MacParameters *parameters);

	void enqueue (Packet *packet);
	void enqueueToHead (Packet *packet);
	Packet *dequeue (void);

	void flush (void);

	bool isEmpty (void);

	int size (void);

	Packet *peekNextPacket (void);

private:
	void cleanup (void);
	typedef std::list< std::pair<Packet *, double> > PacketQueue;
	typedef std::list< std::pair<Packet *, double> >::reverse_iterator PacketQueueRI;
	typedef std::list< std::pair<Packet *, double> >::iterator PacketQueueI;
	PacketQueue m_queue;
	MacParameters *m_parameters;
};


#endif /* MAC_QUEUE_80211E */
