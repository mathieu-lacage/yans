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
#ifndef MAC_NETWORK_INTERFACE_H
#define MAC_NETWORK_INTERFACE_H

#include <stdint.h>
#include "callback.h"
#include "mac-address.h"

namespace yans {

class Packet;

class MacNetworkInterface {
public:
	typedef Callback<void, Packet *, MacNetworkInterface *> RxCallback;

	MacNetworkInterface (MacAddress self, uint16_t mtu);
	virtual ~MacNetworkInterface () = 0;

	MacAddress get_mac_address (void) const;
	uint16_t get_mtu (void) const;
	bool is_down (void) const;
	void set_up   (void);
	void set_down (void);

	void set_rx_callback (RxCallback callback);
	void send (Packet *packet, MacAddress to);
protected:
	void forward_up (Packet *packet);
private:
	virtual void notify_up (void) = 0;
	virtual void notify_down (void) = 0;
	virtual void real_send (Packet *packet, MacAddress to) = 0;

	RxCallback m_rx_callback;
	MacAddress m_self;
	uint16_t m_mtu;
	bool m_is_down;
};

}; // namespace yans

#endif /* MAC_NETWORK_INTERFACE_H */