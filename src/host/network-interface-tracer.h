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

#ifndef NETWORK_INTERFACE_TRACER_H
#define NETWORK_INTERFACE_TRACER_H

#include <stdint.h>
#include "callback.tcc"

namespace yans {

class Host;
class Packet;
class WriteFile;
class Buffer;
class NetworkInterface;

class NetworkInterfaceTracer {
public:
	NetworkInterfaceTracer (Host *host, NetworkInterface *interface);
	~NetworkInterfaceTracer ();

	void trace_tx_mac (Packet *packet);
	void trace_rx_mac (Packet *packet);

	void enable_all (void);
	void disable_all (void);
private:
	void write_buffer_to_file (uint32_t to_write);
	void write_pcap_header (void);
	void write_packet_header (uint32_t size);
	void write_32 (uint32_t data);
	void write_16 (uint16_t data);
	uint32_t write_data (uint8_t *buffer, uint32_t size);

	Host *m_host;
	NetworkInterface *m_interface;
	WriteFile *m_file;
	bool m_enable_all;
	Callback<uint32_t (uint8_t *, uint32_t)> *m_write_callback;
};

}; // namespace yans

#endif /* NETWORK_INTERFACE_TRACER_H */
