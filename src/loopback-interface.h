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
	virtual std::string const *get_name (void);
	virtual MacAddress get_mac_address (void);

	virtual void set_up   (void);
	virtual void set_down (void);
	virtual bool is_down (void);

	virtual void set_ipv4_handler (Ipv4 *ipv4);
	virtual void set_ipv4_address (Ipv4Address address);
	virtual void set_ipv4_mask    (Ipv4Mask mask);
	virtual Ipv4Address get_ipv4_address (void);
	virtual Ipv4Mask    get_ipv4_mask    (void);

	virtual void send (Packet *packet, Ipv4Address to);
private:
	std::string m_name;
	Ipv4 *m_ipv4;
	Ipv4Address m_address;
	Ipv4Mask m_mask;
	uint16_t m_mtu;
	bool m_down;
};


#endif /* LOOPBACK_INTERFACE_H */
