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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "chunk-probe.h"
#include "buffer.h"
#include "simulator.h"

namespace yans {

ChunkProbeRequest::~ChunkProbeRequest ()
{}

void 
ChunkProbeRequest::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
Ssid 
ChunkProbeRequest::get_ssid (void) const
{
	return m_ssid;
}
void 
ChunkProbeRequest::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}

SupportedRates 
ChunkProbeRequest::get_supported_rates (void) const
{
	return m_rates;
}
void 
ChunkProbeRequest::add_to (Buffer *buffer) const
{
	uint32_t size = 0;
	size += m_ssid.get_size ();
	size += m_rates.get_size ();
	
	buffer->add_at_start (size);
	buffer->seek (0);
	m_ssid.write_to (buffer);
	m_rates.write_to (buffer);
}
void 
ChunkProbeRequest::remove_from (Buffer *buffer)
{
	uint32_t size = 0;
	size += m_ssid.read_from (buffer);
	size += m_rates.read_from (buffer);
	buffer->remove_at_start (size);

}
void 
ChunkProbeRequest::print (std::ostream *os) const
{
	//XXX
}

ChunkProbeResponse::ChunkProbeResponse ()
{}
ChunkProbeResponse::~ChunkProbeResponse ()
{}

Ssid 
ChunkProbeResponse::get_ssid (void) const
{
	return m_ssid;
}
uint64_t 
ChunkProbeResponse::get_beacon_interval_us (void) const
{
	return m_beacon_interval;
}

void 
ChunkProbeResponse::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
void 
ChunkProbeResponse::set_beacon_interval_us (uint64_t us)
{
	m_beacon_interval = us;
}

void 
ChunkProbeResponse::add_to (Buffer *buffer) const
{
	// timestamp
	// beacon interval
	// capability information
	// ssid
	// supported rates
	// fh parameter set
	// ds parameter set
	// cf parameter set
	// ibss parameter set
	//XXX
	uint32_t size = 0;
	size += 8; // timestamp
	size += 2; // beacon interval
	size += m_capability.get_size (); // capability information
	size += m_ssid.get_size ();
	size += m_rates.get_size ();
	size += 3; // ds parameter set
	buffer->add_at_start (size);
	buffer->seek (0);
	buffer->write_u64 (Simulator::now_us ());
	buffer->write_u16 (m_beacon_interval / 1024);
	m_capability.write_to (buffer);
	m_ssid.write_to (buffer);
	m_rates.write_to (buffer);
	buffer->skip (3); // ds parameter set
}
void 
ChunkProbeResponse::remove_from (Buffer *buffer)
{
	buffer->seek (0);
	uint32_t size = 0;
	size += 8; // timestamp
	size += 2; // beacon interval
	size += m_capability.read_from (buffer);
	size += m_ssid.read_from (buffer);
	size += m_rates.read_from (buffer);
	size += 3; // ds parameter set
	buffer->remove_at_start (size);
}
void 
ChunkProbeResponse::print (std::ostream *os) const
{
	//XXX
}



}; // namespace yans
