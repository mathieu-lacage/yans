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

#ifndef TCP_TX_BUFFER_H
#define TCP_TX_BUFFER_H

#include <stdint.h>
#include <list>

class Packet;

class TcpTxBuffer {
public:
	TcpTxBuffer ();

	void set_size (uint32_t size);
	uint32_t get_size_left (void);

	void add (Packet *packet);
	Packet *remove (uint32_t size);
private:
	typedef std::list<Packet *> Data;
	typedef std::list<Packet *>::iterator DataI;

	Data m_data;
	uint32_t m_chunk_start;
	uint32_t m_size;
	uint32_t m_used;
};

#endif /* TCP_TX_BUFFER_H */
