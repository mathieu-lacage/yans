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
#include "yans/simulator.h"
#include <cassert>

namespace {
using namespace yans;

uint32_t
get_size (CapabilityInformation const &self)
{
	return 2;
}
Buffer::Iterator
write_to (Buffer::Iterator i, CapabilityInformation const&self)
{
	//XXX
	i.next (get_size (self));
	return i;
}
Buffer::Iterator
read_from (Buffer::Iterator i, CapabilityInformation &self)
{
	//XXX
	i.next (get_size (self));
	return i;
}

uint32_t 
get_size (Ssid const&ssid)
{
	return ssid.get_length () + 1 + 1;
}
Buffer::Iterator
write_to (Buffer::Iterator i, Ssid const &self)
{
	i.write_u8 (0); // ssid element id
	uint32_t size = self.get_length ();
	i.write_u8 (size);
	uint8_t ssid[32];
	self.peek (ssid);
	i.write (ssid, size);
	return i;
}
Buffer::Iterator
read_from (Buffer::Iterator i, Ssid &self)
{
	//uint8_t element_id = buffer->read_u8 ();
	//assert (element_id == 0);
	i.read_u8 ();
	uint8_t size = i.read_u8 ();
	assert (size <= 32);
	uint8_t ssid[32];
	i.read (ssid, size);
	self.set (ssid, size);
	return i;
}


uint32_t
get_size (StatusCode const&self)
{
	return 2;
}
Buffer::Iterator
write_to (Buffer::Iterator i, StatusCode const&self)
{
	i.write_hton_u16 (self.peek_code ());
	return i;
}
Buffer::Iterator
read_from (Buffer::Iterator i, StatusCode &self)
{
	self.set_code (i.read_ntoh_u16 ());
	return i;
}


uint32_t
get_size (SupportedRates const&rates)
{
	return rates.get_n_rates () + 1 + 1;
}
Buffer::Iterator
write_to (Buffer::Iterator i, SupportedRates const &self)
{
	i.write_u8 (1); // supported rates element id
	i.write_u8 (self.get_n_rates ());
	uint8_t rates[8];
	self.peek_rates (rates);
	i.write (rates, self.get_n_rates ());
	return i;
}
Buffer::Iterator
read_from (Buffer::Iterator i, SupportedRates &self)
{
	//uint8_t rates_id = buffer->read_u8 ();
	//assert (rates_id == 1);
	i.read_u8 ();
	uint8_t n_rates = i.read_u8 ();
	assert (n_rates <= 8);
	uint8_t rates[8];
	i.read (rates, n_rates);
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
uint32_t
ChunkMgtProbeRequest::get_size (void) const
{
	uint32_t size = 0;
	size += ::get_size (m_ssid);
	size += ::get_size (m_rates);
	return size;
}
void 
ChunkMgtProbeRequest::print (std::ostream *os) const
{
	//XXX
}
void 
ChunkMgtProbeRequest::add_to (Buffer *buffer) const
{	
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i = write_to (i, m_ssid);
	i = write_to (i, m_rates);
}
void 
ChunkMgtProbeRequest::peek_from (Buffer const *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i = read_from (i, m_ssid);
	i = read_from (i, m_rates);
	m_read_size = buffer->begin ().get_distance_from (i);
}
void 
ChunkMgtProbeRequest::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (m_read_size);
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
SupportedRates 
ChunkMgtProbeResponse::get_supported_rates (void) const
{
	return m_rates;
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
ChunkMgtProbeResponse::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}
uint32_t
ChunkMgtProbeResponse::get_size (void) const
{
	uint32_t size = 0;
	size += 8; // timestamp
	size += 2; // beacon interval
	size += ::get_size (m_capability); // capability information
	size += ::get_size (m_ssid);
	size += ::get_size (m_rates);
	size += 3; // ds parameter set
	return size;
}
void 
ChunkMgtProbeResponse::print (std::ostream *os) const
{
	//XXX
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
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i.write_u64 (Simulator::now_us ());
	i.write_hton_u16 (m_beacon_interval / 1024);
	i = write_to (i, m_capability);
	i = write_to (i, m_ssid);
	i = write_to (i, m_rates);
	i.next (3); // ds parameter set.
}
void 
ChunkMgtProbeResponse::peek_from (Buffer const *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i.next (8); // timestamp
	m_beacon_interval = i.read_ntoh_u16 ();
	m_beacon_interval *= 1024;
	i = read_from (i, m_capability);
	i = read_from (i, m_ssid);
	i = read_from (i, m_rates);
	i.next (3); // ds parameter set
	m_read_size = buffer->begin ().get_distance_from (i);
}
void 
ChunkMgtProbeResponse::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (m_read_size);
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
uint32_t 
ChunkMgtAssocRequest::get_size (void) const
{
	uint32_t size = 0;
	size += ::get_size (m_capability);
	size += 2;
	size += ::get_size (m_ssid);
	size += ::get_size (m_rates);
	return size;
}
void 
ChunkMgtAssocRequest::print (std::ostream *os) const
{
	//XXX
}
void 
ChunkMgtAssocRequest::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i = write_to (i, m_capability);
	i.write_hton_u16 (m_listen_interval);
	i = write_to (i, m_ssid);
	i = write_to (i, m_rates);
}
void 
ChunkMgtAssocRequest::peek_from (Buffer const *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i = read_from (i, m_capability);
	m_listen_interval = i.read_ntoh_u16 ();
	i = read_from (i, m_ssid);
	i = read_from (i, m_rates);
	m_read_size = buffer->begin ().get_distance_from (i);
}
void 
ChunkMgtAssocRequest::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (m_read_size);
}

ChunkMgtAssocResponse::ChunkMgtAssocResponse ()
{}
ChunkMgtAssocResponse::~ChunkMgtAssocResponse ()
{}

StatusCode 
ChunkMgtAssocResponse::get_status_code (void)
{
	return m_code;
}
void 
ChunkMgtAssocResponse::set_status_code (StatusCode code)
{
	m_code = code;
}

uint32_t
ChunkMgtAssocResponse::get_size (void) const
{
	uint32_t size = 0;
	size += ::get_size (m_capability);
	size += ::get_size (m_code);
	size += 2; // aid
	size += ::get_size (m_rates);
	return size;
}

void 
ChunkMgtAssocResponse::print (std::ostream *os) const
{
	//XXX
}
void 
ChunkMgtAssocResponse::add_to (Buffer *buffer) const
{
	buffer->add_at_start (get_size ());
	Buffer::Iterator i = buffer->begin ();
	i = write_to (i, m_capability);
	i = write_to (i, m_code);
	i.next (2);
	i = write_to (i, m_rates);
}
void 
ChunkMgtAssocResponse::peek_from (Buffer const *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	i = read_from (i, m_capability);
	i = read_from (i, m_code);
	m_aid = i.read_ntoh_u16 ();
	i = read_from (i, m_rates);

	m_read_size = buffer->begin ().get_distance_from (i);
}
void 
ChunkMgtAssocResponse::remove_from (Buffer *buffer)
{
	buffer->remove_at_start (m_read_size);
}

}; // namespace yans
