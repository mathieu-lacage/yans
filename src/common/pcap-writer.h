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

#ifndef PCAP_WRITER_H
#define PCAP_WRITER_H

#include "callback.tcc"
#include <stdint.h>

namespace yans {

class DataWriter;
class Packet;

class PcapWriter {
public:
	PcapWriter ();
	~PcapWriter ();

	void open (char const *name);

	void write_header_ethernet (void);

	void write_packet (Packet *packet);

private:
	void write_data (uint8_t *buffer, uint32_t size);
	void write_32 (uint32_t data);
	void write_16 (uint16_t data);
	DataWriter *m_writer;
	Callback<void (uint8_t *, uint32_t)> *m_write_callback;
};

}; // namespace yans

#endif /* PCAP_WRITER_H */
