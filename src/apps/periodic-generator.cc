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

#include "periodic-generator.h"
#include "simulator.h"
#include "packet.h"
#include "chunk-constant-data.h"
#include "event.h"
#include "event.tcc"
#include <cassert>


namespace yans {

PeriodicGenerator::PeriodicGenerator ()
	: m_stop_at_us (0),
	  m_n (0),
	  m_current_event ()
{}

PeriodicGenerator::~PeriodicGenerator ()
{}

void 
PeriodicGenerator::set_send_callback (GeneratorCallback callback)
{
	m_callback = callback;
}

void 
PeriodicGenerator::set_packet_interval (double interval)
{
	m_interval_us = (uint64_t)(interval * 1000000);
}
void 
PeriodicGenerator::set_packet_size (uint16_t size)
{
	m_size = size;
}

void 
PeriodicGenerator::start_now (void)
{
	assert (!m_current_event.is_running ());
	m_current_event = make_event (&PeriodicGenerator::send_next_packet, this);
	Simulator::insert_in_us (m_interval_us, m_current_event);
}
void 
PeriodicGenerator::stop_now (void)
{
	m_current_event.cancel ();
}


void 
PeriodicGenerator::start_at (double start)
{
	assert (!m_current_event.is_running ());
	m_current_event = make_event (&PeriodicGenerator::send_next_packet, this);
	Simulator::insert_at_s (start, m_current_event);
}
void 
PeriodicGenerator::stop_at (double end)
{
	m_stop_at_us = (uint64_t)(end * 1000000);
}


void 
PeriodicGenerator::send_next_packet (void)
{
	/* stop packet transmissions.*/
	if (m_stop_at_us > 0 && Simulator::now_us () >= m_stop_at_us) {
		return;
	}
	/* schedule next packet transmission. */
	m_current_event = make_event (&PeriodicGenerator::send_next_packet, this);
	Simulator::insert_in_us (m_interval_us, m_current_event);
	/* create packet. */
	Packet *packet = PacketFactory::create ();
	ChunkConstantData data (m_size, m_n);
	packet->add (&data);
	m_callback (packet);
	packet->unref ();
	m_n++;
}

}; //namespace yans
