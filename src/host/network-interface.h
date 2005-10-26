/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include <string>
#include "mac-address.h"
#include "ipv4-address.h"

class Packet;
class Ipv4;
class Host;
class NetworkInterfaceTracer;

class NetworkInterface {
public:
	virtual ~NetworkInterface () = 0;

	virtual void set_host (Host *host) = 0;
	virtual NetworkInterfaceTracer *get_tracer (void) = 0;

	virtual void set_mac_address (MacAddress self) = 0;
	virtual MacAddress get_mac_address (void) = 0;
	virtual std::string const *get_name (void) = 0;
	/* needed to decide whether or not to perform
	 * ip fragmentation when sending a packet on
	 * this interface.
	 */
	virtual uint16_t get_mtu (void) = 0;

	virtual void set_up   (void) = 0;
	virtual void set_down (void) = 0;
	virtual bool is_down (void) = 0;

	/* methods specific to ipv4. */
	virtual void set_ipv4_handler (Ipv4 *ipv4) = 0;
	virtual void set_ipv4_address (Ipv4Address address) = 0;
	virtual void set_ipv4_mask    (Ipv4Mask mask) = 0;
	virtual Ipv4Address get_ipv4_address (void) = 0;
	virtual Ipv4Mask    get_ipv4_mask    (void) = 0;

	virtual void send (Packet *packet, Ipv4Address dest) = 0;
};



#endif /* NETWORK_INTERFACE_H */
