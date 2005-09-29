/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "mac-address.h"
#include "ipv4-address.h"

class Packet;
class Ipv4;

class NetworkInterface {
public:
	virtual ~NetworkInterface ();

	/* needed to decide whether or not to perform
	 * ip fragmentation when sending a packet on
	 * this interface.
	 */
	virtual uint16_t get_mtu (void) = 0;
	virtual Ipv4Address get_ipv4_address (void) = 0;
	virtual Ipv4Mask get_ipv4_mask (void) = 0;
	virtual MacAddress get_mac_address (void) = 0;

	virtual void register_ipv4_handler (Ipv4 *ipv4) = 0;

	virtual void send_ipv4 (Packet *packet) = 0;
};



#endif /* NETWORK_INTERFACE_H */
