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

#include <string>
#include "write-file.h"
#include "network-interface-tracer.h"
#include "network-interface.h"
#include "packet.h"
#include "buffer.h"
#include "simulator.h"

NetworkInterfaceTracer::NetworkInterfaceTracer (Host *host, NetworkInterface *interface)
	: m_host (host),
	  m_interface (interface),
	  m_enable_all (false)
{
	std::string *filename = new std::string ("/logs/");
	filename->append (*(interface->get_name ()));
	m_file = new WriteFile (host);
	m_file->open (filename);
	delete filename;
	m_buffer = new Buffer (64);
	write_pcap_header ();
}
NetworkInterfaceTracer::~NetworkInterfaceTracer ()
{
	m_file->close ();
	delete m_file;
	m_file = (WriteFile *)0xdeadbeaf;
	delete m_buffer;
	m_buffer = (Buffer *)0xdeadbeaf;
}

void
NetworkInterfaceTracer::write_32 (uint32_t data)
{
	m_file->write ((uint8_t*)&data, 4);
}
void
NetworkInterfaceTracer::write_16 (uint16_t data)
{
	m_file->write ((uint8_t*)&data, 2);
}

void
NetworkInterfaceTracer::write_pcap_header (void)
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
NetworkInterfaceTracer::write_packet_header (uint32_t size)
{
	uint64_t current = Simulator::now_us ();
	write_32 ((current >> 32) & 0xffffffff);
	write_32 (current & 0xffffffff);
	write_32 (size);
	write_32 (size);
}

void
NetworkInterfaceTracer::write_buffer_to_file (uint32_t to_write)
{
	uint8_t *data = m_buffer->peek_data ();
	uint32_t written = 0;
	while (written < to_write) {
		uint32_t just_written = m_file->write (data, to_write - written);
		written += just_written;
		data += just_written;
	}
}

void 
NetworkInterfaceTracer::trace_tx_mac (Packet *packet)
{
	if (!m_enable_all) {
		return;
	}
	m_buffer->seek (0);
	uint32_t serialized = packet->serialize (m_buffer);
	write_packet_header (serialized);
	write_buffer_to_file (serialized);
}
void 
NetworkInterfaceTracer::trace_rx_mac (Packet *packet)
{
	if (!m_enable_all) {
		return;
	}
	m_buffer->seek (0);
	uint32_t serialized = packet->serialize (m_buffer);
	write_packet_header (serialized);
	write_buffer_to_file (serialized);
}

void 
NetworkInterfaceTracer::enable_all (void)
{
	m_enable_all = true;
}
void 
NetworkInterfaceTracer::disable_all (void)
{
	m_enable_all = false;
}

