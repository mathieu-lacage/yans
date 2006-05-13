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
#include <cassert>

namespace {
using namespace yans;

Buffer::Iterator
write_to (CapabilityInformation const&self, Buffer::Iterator i)
{
	//XXX
	return i;
}
Buffer::Iterator
read_from (CapabilityInformation &self, Buffer::Iterator i)
{
	//XXX
	return i;
}
 
Buffer::Iterator
write_to (Ssid const &self, Buffer::Iterator i)
{
	i.write_u8 (0); // ssid element id
	i.next ();
	uint32_t size = self.get_size ();
	i.write_u8 (size);
	i.next ();
	uint8_t ssid[32];
	self.peek (ssid);
	i.write (ssid, size);
	i.next (size);
	return i;
}
Buffer::Iterator
read_from (Ssid &self, Buffer::Iterator i)
{
	//uint8_t element_id = buffer->read_u8 ();
	//assert (element_id == 0);
	i.read_u8 ();
	i.next ();
	uint8_t size = i.read_u8 ();
	i.next ();
	assert (size <= 32);
	uint8_t ssid[32];
	i.read (ssid, size);
	self.set (ssid);
	i.next (size);
	return i;
}
Buffer::Iterator
write_to (StatusCode const&self, Buffer::Iterator i)
{
	i.write_hton_u16 (self.peek_code ());
	i.next (2);
	return i;
}
Buffer::Iterator
read_from (StatusCode &self, Buffer::Iterator i)
{
	self.set_code (i.read_ntoh_u16 ());
	i.next (2);
	return i;
}
Buffer::Iterator
write_to (SupportedRates const &self, Buffer::Iterator i)
{
	i.write_u8 (1); // supported rates element id
	i.next ();
	i.write_u8 (self.get_n_rates ());
	i.next ();
	uint8_t rates[8];
	self.peek_rates (rates);
	i.write (rates, self.get_n_rates ());
	i.next (self.get_n_rates ());
	return i;
}
Buffer::Iterator
read_from (SupportedRates &self, Buffer::Iterator i)
{
	//uint8_t rates_id = buffer->read_u8 ();
	//assert (rates_id == 1);
	i.read_u8 ();
	i.next ();
	uint8_t n_rates = i.read_u8 ();
	i.next ();
	assert (n_rates <= 8);
	uint8_t rates[8];
	i.read (rates, n_rates);
	i.next (n_rates);
	self.set_rates (rates, n_rates);
	return i;
}

};

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
	Buffer::Iterator i = buffer->begin ();
	i = write_to (m_ssid, i);
	i = write_to (m_rates, i);
}
void 
ChunkMgtProbeRequest::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i = read_from (m_ssid, i);
	i = read_from (m_rates, i);
	uint32_t size = buffer->begin ().get_distance_from (i);
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
	Buffer::Iterator i = buffer->begin ();
	i.write_u64 (Simulator::now_us ());
	i.next (8);
	i.write_u16 (m_beacon_interval / 1024);
	i.next (2);
	i = write_to (m_capability, i);
	i = write_to (m_ssid, i);
	i = write_to (m_rates, i);
	i.next (3); // ds parameter set.
}
void 
ChunkMgtProbeResponse::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i.next (8+2); // timestamp + beacon interval
	i = read_from (m_capability, i);
	i = read_from (m_ssid, i);
	i = read_from (m_rates, i);
	i.next (3); // ds parameter set
	uint32_t size = buffer->begin ().get_distance_from (i);
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
	Buffer::Iterator i = buffer->begin ();
	i = write_to (m_capability, i);
	i.write_hton_u16 (m_listen_interval);
	i.next (2);
	i = write_to (m_ssid, i);
	i = write_to (m_rates, i);
}
void 
ChunkMgtAssocRequest::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i = read_from (m_capability, i);
	m_listen_interval = i.read_ntoh_u16 ();
	i.next (2);
	i = read_from (m_ssid, i);
	i = read_from (m_rates, i);
	uint32_t size = buffer->begin ().get_distance_from (i);
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
	Buffer::Iterator i = buffer->begin ();
	i = write_to (m_capability, i);
	i = write_to (m_code, i);
	i.next (2);
	i = write_to (m_rates, i);
}
void 
ChunkMgtAssocResponse::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i = read_from (m_capability, i);
	i = read_from (m_code, i);
	m_aid = i.read_ntoh_u16 ();
	i.next (2);
	i = read_from (m_rates, i);

	uint32_t size = buffer->begin ().get_distance_from (i);
	buffer->remove_at_start (size);
}
void 
ChunkMgtAssocResponse::print (std::ostream *os) const
{
	//XXX
}

}; // namespace yans
