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
#include "chunk-assoc.h"
#include "buffer.h"

namespace yans {

ChunkAssocRequest::ChunkAssocRequest ()
{}
ChunkAssocRequest::~ChunkAssocRequest ()
{}

void 
ChunkAssocRequest::set_ssid (Ssid ssid)
{
	m_ssid = ssid;
}
void 
ChunkAssocRequest::set_supported_rates (SupportedRates rates)
{
	m_rates = rates;
}
void
ChunkAssocRequest::set_listen_interval (uint16_t interval)
{
	m_listen_interval = interval;
}
Ssid 
ChunkAssocRequest::get_ssid (void) const
{
	return m_ssid;
}
SupportedRates 
ChunkAssocRequest::get_supported_rates (void) const
{
	return m_rates;
}
uint16_t
ChunkAssocRequest::get_listen_interval (void) const
{
	return m_listen_interval;
}

void 
ChunkAssocRequest::add_to (Buffer *buffer) const
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
ChunkAssocRequest::remove_from (Buffer *buffer)
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
ChunkAssocRequest::print (std::ostream *os) const
{
	//XXX
}

ChunkAssocResponse::ChunkAssocResponse ()
{}
ChunkAssocResponse::~ChunkAssocResponse ()
{}

bool 
ChunkAssocResponse::is_success (void)
{
	return m_code.is_success ();
}

void 
ChunkAssocResponse::add_to (Buffer *buffer) const
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
ChunkAssocResponse::remove_from (Buffer *buffer)
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
ChunkAssocResponse::print (std::ostream *os) const
{
	//XXX
}

}; // namespace yans

