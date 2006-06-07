/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */
#include "parallel-channel-80211.h"
#include "yans/simulator.h"
#include "yans/propagation-model.h"
#include "yans/event.tcc"
#include "yans/packet.h"
#include "yans/count-ptr-holder.tcc"

namespace {
void
forward_up (yans::CountPtrHolder<yans::Packet const> p, double rx_power, uint8_t tx_mode, uint8_t stuff,
	    yans::PropagationModel *propagation)
{
	yans::Packet const*packet = p.remove ();
	propagation->receive (packet, rx_power, tx_mode, stuff);
	packet->unref ();
}
}


class Channel80211Queue : public yans::ParallelSimulatorQueue {
public:
	Channel80211Queue (ParallelChannel80211 *channel);
	~Channel80211Queue ();
private:
	void send_null_message (void);
	ParallelChannel80211 *m_channel;
};

Channel80211Queue::Channel80211Queue (ParallelChannel80211 *channel)
	: m_channel (channel)
{}

Channel80211Queue::~Channel80211Queue ()
{}

void
Channel80211Queue::send_null_message (void)
{
	m_channel->send_null_message ();
}

ParallelChannel80211::ParallelChannel80211 ()
{
	m_queue = new Channel80211Queue (this);
	yans::Simulator::add_parallel_queue (m_queue);
}
ParallelChannel80211::~ParallelChannel80211 ()
{
	delete m_queue;
}
void 
ParallelChannel80211::add (Remote::Channel80211_var channel)
{
	m_channels.push_back (channel);
}
void 
ParallelChannel80211::receive (yans::Packet const *packet, double tx_power_dbm,
			       double from_x, double from_y, double from_z,
			       uint8_t tx_mode, uint8_t stuff)
{
	for (ModelsCI i = m_models.begin (); i != m_models.end (); i++) {
		uint64_t delay_us = (*i)->get_delay_us (from_x, from_y, from_z);
		double rx_power_w = (*i)->get_rx_power_w (tx_power_dbm, from_x, from_y, from_z);
		m_queue->insert_at_us (delay_us, yans::make_event (&forward_up, 
								   yans::make_count_ptr_holder (packet),
								   rx_power_w, tx_mode, stuff, *i));
	}
}
void 
ParallelChannel80211::real_add (yans::PropagationModel *model)
{
	m_models.push_back (model);
}
void 
ParallelChannel80211::real_send (yans::Packet const *packet, double tx_power_dbm,
				 uint8_t tx_mode, uint8_t stuff, 
				 yans::PropagationModel const*caller) const
{
	double from_x, from_y, from_z;
	caller->get_position (from_x,from_y,from_z);
	for (ModelsCI i = m_models.begin (); i != m_models.end (); i++) {
		if (caller != (*i)) {
			uint64_t delay_us = (*i)->get_delay_us (from_x, from_y, from_z);
			double rx_power_w = (*i)->get_rx_power_w (tx_power_dbm, from_x, from_y, from_z);
			yans::Simulator::insert_in_us (delay_us, yans::make_event (&forward_up, 
										   yans::make_count_ptr_holder (packet),
										   rx_power_w, tx_mode, stuff, *i));
		}
	}
	struct Remote::SourcePosition position;
	position.x = from_x;
	position.y = from_y;
	position.z = from_z;
	uint8_t *data = packet->peek_data ();
	Remote::Buffer buffer = Remote::Buffer (packet->get_size (),
						packet->get_size (),
						data, 
						0 /* make sure we (the caller) keep ownership
						     of the data buffer. */);
	
	for (ChannelsCI j = m_channels.begin (); j != m_channels.end (); j++) {
		(*j)->receive (position, yans::Simulator::now_us (), buffer,
			       tx_power_dbm, tx_mode, stuff);
	}
}

void
ParallelChannel80211::send_null_message (void)
{
	Remote::SourcePositions_var sources;
	Remote::SourcePosition position;
	uint32_t index = 0;
	for (ModelsCI i = m_models.begin (); i != m_models.end (); i++) {
		(*i)->get_position (position.x, position.y, position.z);
		sources[index] = position;
		index++;
	}
	for (ChannelsI j = m_channels.begin (); j != m_channels.end (); j++) {
		(*j)->receive_null (yans::Simulator::now_us (), sources);
	}
}
