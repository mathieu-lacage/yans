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

#ifndef SOCKET_TCP_CONNECTED_H
#define SOCKET_TCP_CONNECTED_H

#include <stdint.h>
#include "ipv4-address.h"

class SocketTcpConnected {
 public:
	Ipv4Address get_peer_ipv4_address (void);
	uint16_t get_peer_port (void);
	
	uint32_t send (uint8_t const *buf, uint32_t length, int flags);
	uint32_t recv (uint8_t *buf, uint32_t length, int flags);
};

#endif /* SOCKET_TCP_CONNECTED_H */
