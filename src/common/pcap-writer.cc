/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#include "pcap-writer.h"
#include "callback.tcc"
#include "simulator.h"
#include "packet.h"
#include "data-writer.h"

namespace yans {

PcapWriter::PcapWriter ()
{
	m_write_callback = make_callback (&PcapWriter::write_data, this);
}
PcapWriter::~PcapWriter ()
{
	delete m_write_callback;
}

void 
PcapWriter::set_data_writer (DataWriter *writer)
{
	m_writer = writer;
}

void 
PcapWriter::write_header_ethernet (void)
{
	write_32 (0xa1b2c3d4);
	write_16 (2);
	write_16 (4);
	write_32 (0);
	write_32 (0);
	write_32 (0xffff);
	write_32 (1);
}

void 
PcapWriter::write_packet (Packet *packet)
{
	uint64_t current = Simulator::now_us ();
	write_32 ((current >> 32) & 0xffffffff);
	write_32 (current & 0xffffffff);
	write_32 (packet->get_size ());
	write_32 (packet->get_size ());
	packet->write (m_write_callback);
}

void
PcapWriter::write_data (uint8_t *buffer, uint32_t size)
{
	m_writer->write (buffer, size);
}
void
PcapWriter::write_32 (uint32_t data)
{
	m_writer->write ((uint8_t*)&data, 4);
}
void
PcapWriter::write_16 (uint16_t data)
{
	m_writer->write ((uint8_t*)&data, 2);
}

}; // namespace yans
