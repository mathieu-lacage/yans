/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef LOOPBACK_INTERFACE_H
#define LOOPBACK_INTERFACE_H

#include <stdint.h>

#include "network-interface.h"
#include "ipv4-address.h"

class LoopbackInterface : public NetworkInterface {
public:
	LoopbackInterface ();
	virtual ~LoopbackInterface ();

	virtual uint16_t get_mtu (void);
	virtual MacAddress get_mac_address (void);

	virtual void set_ipv4_handler (Ipv4 *ipv4);
	virtual Ipv4Address get_ipv4_address (void);
	virtual Ipv4Mask get_ipv4_mask (void);
	virtual void set_ipv4_next_hop (Ipv4Address next_hop);

	virtual void send (Packet *packet);
private:
	Ipv4 *m_ipv4;
	Ipv4Address m_address;
	Ipv4Mask m_mask;
	uint16_t m_mtu;
};


#endif /* LOOPBACK_INTERFACE_H */
