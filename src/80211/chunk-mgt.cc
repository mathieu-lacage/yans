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
#include "chunk-mgt.h"
#include "buffer.h"
#include "simulator.h"

namespace yans {


ChunkMgtProbeRequest::~ChunkMgtProbeRequest ()
{}

void 
ChunkMgtProbeRequest::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
Ssid 
ChunkMgtProbeRequest::get_ssid (void) const
{
	return m_ssid;
}
void 
ChunkMgtProbeRequest::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}

SupportedRates 
ChunkMgtProbeRequest::get_supported_rates (void) const
{
	return m_rates;
}
void 
ChunkMgtProbeRequest::add_to (Buffer *buffer) const
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
ChunkMgtProbeRequest::remove_from (Buffer *buffer)
{
	uint32_t size = 0;
	size += m_ssid.read_from (buffer);
	size += m_rates.read_from (buffer);
	buffer->remove_at_start (size);

}
void 
ChunkMgtProbeRequest::print (std::ostream *os) const
{
	//XXX
}

ChunkMgtProbeResponse::ChunkMgtProbeResponse ()
{}
ChunkMgtProbeResponse::~ChunkMgtProbeResponse ()
{}

Ssid 
ChunkMgtProbeResponse::get_ssid (void) const
{
	return m_ssid;
}
uint64_t 
ChunkMgtProbeResponse::get_beacon_interval_us (void) const
{
	return m_beacon_interval;
}

void 
ChunkMgtProbeResponse::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
void 
ChunkMgtProbeResponse::set_beacon_interval_us (uint64_t us)
{
	m_beacon_interval = us;
}

void 
ChunkMgtProbeResponse::add_to (Buffer *buffer) const
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
ChunkMgtProbeResponse::remove_from (Buffer *buffer)
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
ChunkMgtProbeResponse::print (std::ostream *os) const
{
	//XXX
}


ChunkMgtAssocRequest::ChunkMgtAssocRequest ()
{}
ChunkMgtAssocRequest::~ChunkMgtAssocRequest ()
{}

void 
ChunkMgtAssocRequest::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
void 
ChunkMgtAssocRequest::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}
void
ChunkMgtAssocRequest::set_listen_interval (uint16_t interval)
{
	m_listen_interval = interval;
}
Ssid 
ChunkMgtAssocRequest::get_ssid (void) const
{
	return m_ssid;
}
SupportedRates 
ChunkMgtAssocRequest::get_supported_rates (void) const
{
	return m_rates;
}
uint16_t
ChunkMgtAssocRequest::get_listen_interval (void) const
{
	return m_listen_interval;
}

void 
ChunkMgtAssocRequest::add_to (Buffer *buffer) const
{
	uint32_t size = 0;
	size += m_capability.get_size ();
	size += 2;
	size += m_ssid.get_size ();
	size += m_rates.get_size ();
	
	buffer->add_at_start (size);
	buffer->seek (0);
	m_capability.write_to (buffer);
	buffer->write_hton_u16 (m_listen_interval);
	m_ssid.write_to (buffer);
	m_rates.write_to (buffer);
}
void 
ChunkMgtAssocRequest::remove_from (Buffer *buffer)
{
	uint32_t size = 0;
	size += m_capability.read_from (buffer);
	m_listen_interval = buffer->read_ntoh_u16 ();
	size += 2;
	size += m_ssid.read_from (buffer);
	size += m_rates.read_from (buffer);
	buffer->remove_at_start (size);
}
void 
ChunkMgtAssocRequest::print (std::ostream *os) const
{
	//XXX
}

ChunkMgtAssocResponse::ChunkMgtAssocResponse ()
{}
ChunkMgtAssocResponse::~ChunkMgtAssocResponse ()
{}

bool 
ChunkMgtAssocResponse::is_success (void)
{
	return m_code.is_success ();
}

void 
ChunkMgtAssocResponse::add_to (Buffer *buffer) const
{
	uint32_t size = 0;
	size += m_capability.get_size ();
	size += m_code.get_size ();
	size += 2; // aid
	size += m_rates.get_size ();

	buffer->add_at_start (size);
	buffer->seek (0);
	m_capability.write_to (buffer);
	m_code.write_to (buffer);
	buffer->skip (2);
	m_rates.write_to (buffer);
}
void 
ChunkMgtAssocResponse::remove_from (Buffer *buffer)
{
	uint32_t size = 0;
	size += m_capability.read_from (buffer);
	size += m_code.read_from (buffer);
	m_aid = buffer->read_ntoh_u16 ();
	size += 2;
	size += m_rates.read_from (buffer);

	buffer->remove_at_start (size);
}
void 
ChunkMgtAssocResponse::print (std::ostream *os) const
{
	//XXX
}

}; // namespace yans
