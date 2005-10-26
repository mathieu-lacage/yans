/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef ETHERNET_NETWORK_INTERFACE_H
#define ETHERNET_NETWORK_INTERFACE_H

#include "network-interface.h"

class Arp;
class EthernetArpMacSender;
class Cable;

class EthernetNetworkInterface : public NetworkInterface {
public:
	EthernetNetworkInterface ();
	virtual ~EthernetNetworkInterface ();

	virtual void set_host (Host *host);
	virtual NetworkInterfaceTracer *get_tracer (void);

	virtual void set_mac_address (MacAddress self);
	virtual MacAddress get_mac_address (void);
	virtual std::string const *get_name (void);
	virtual uint16_t get_mtu (void);

	virtual void set_up   (void);
	virtual void set_down (void);
	virtual bool is_down (void);

	virtual void set_ipv4_handler (Ipv4 *ipv4);
	virtual void set_ipv4_address (Ipv4Address address);
	virtual void set_ipv4_mask    (Ipv4Mask mask);
	virtual Ipv4Address get_ipv4_address (void);
	virtual Ipv4Mask    get_ipv4_mask    (void);

	virtual void send (Packet *packet, Ipv4Address dest);

	void connect_to (Cable *cable);
	void recv (Packet *packet);

 private:
	void send_data (Packet *packet, MacAddress dest);
	void send_arp (Packet *packet, MacAddress dest);

	enum {
		ETHER_TYPE_IPV4 = 0x0800,
		ETHER_TYPE_ARP  = 0x0806
	};

	friend class EthernetArpMacSender;

	Arp *m_arp;
	EthernetArpMacSender *m_arp_sender;
	MacAddress m_mac_address;
	Ipv4Address m_ipv4_address;
	Ipv4Mask m_ipv4_mask;
	Ipv4 *m_ipv4;
	std::string *m_name;
	bool m_up;
	Cable *m_cable;
	Host *m_host;
	NetworkInterfaceTracer *m_tracer;
};



#endif /* ETHERNET_NETWORK_INTERFACE_H */
