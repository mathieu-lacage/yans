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

#include <deque>
#include <utility>
#include "chunk-mac-80211-hdr.h"
#include "packet.h"

namespace yans {

class MacParameters;

class MacQueue80211e {
public:
	MacQueue80211e ();
	~MacQueue80211e ();

	void set_max_size (uint32_t max_size);
	void set_max_delay_us (uint64_t us);

	void enqueue (Packet packet, ChunkMac80211Hdr const &hdr);
	Packet dequeue (ChunkMac80211Hdr *hdr, bool *found);

	void flush (void);

	bool is_empty (void);
	uint32_t get_size (void);

private:
	void cleanup (void);
	struct Item {
		Item (Packet packet, 
		      ChunkMac80211Hdr const&hdr, 
		      uint64_t tstamp);
		Packet packet;
		ChunkMac80211Hdr hdr;
		uint64_t tstamp;
	};
	typedef std::deque<struct Item> PacketQueue;
	typedef std::deque<struct Item>::reverse_iterator PacketQueueRI;
	typedef std::deque<struct Item>::iterator PacketQueueI;
	PacketQueue m_queue;
	MacParameters *m_parameters;
	uint32_t m_size;
	uint32_t m_max_size;
	uint64_t m_max_delay_us;
};

}; // namespace yans


#endif /* MAC_QUEUE_80211E */
