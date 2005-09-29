/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "mac-address.h"

class NetworkInterface {
public:
	NetworkInterface ();
	virtual ~NetworkInterface ();

	/* needed to decide whether or not to perform
	 * ip fragmentation when sending a packet on
	 * this interface.
	 */
	virtual uint16_t get_mtu (void) = 0;
	virtual uint32_t get_ipv4_address (void) = 0;
	virtual uint32_t get_ipv4_mask (void) = 0;
	virtual MacAddress get_mac_address (void) = 0;


	virtual void set_destination (MacAddress address);
	virtual void send (Packet *packet) = 0;
};



#endif /* NETWORK_INTERFACE_H */
