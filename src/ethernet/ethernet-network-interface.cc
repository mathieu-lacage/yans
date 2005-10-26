/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ethernet-network-interface.h"
#include "cable.h"
#include "arp.h"
#include "chunk-mac-llc-snap.h"
#include "chunk-mac-crc.h"
#include "packet.h"
#include "ipv4.h"
#include "host.h"
#include "network-interface-tracer.h"

class EthernetArpMacSender : public ArpMacSender {
public:
	EthernetArpMacSender (EthernetNetworkInterface *interface);
	virtual ~EthernetArpMacSender ();
	virtual void send_data (Packet *packet, MacAddress dest);
	virtual void send_arp (Packet *packet, MacAddress dest);
private:
	EthernetNetworkInterface *m_interface;
};

EthernetArpMacSender::EthernetArpMacSender (EthernetNetworkInterface *interface)
	: m_interface (interface)
{}
EthernetArpMacSender::~EthernetArpMacSender ()
{
	m_interface = (EthernetNetworkInterface *)0xdeadbeaf;
}
void 
EthernetArpMacSender::send_data (Packet *packet, MacAddress dest)
{
	m_interface->send_data (packet, dest);
}
void 
EthernetArpMacSender::send_arp (Packet *packet, MacAddress dest)
{
	m_interface->send_arp (packet, dest);
}


EthernetNetworkInterface::EthernetNetworkInterface ()
	: m_name (new std::string ("eth0"))
{
	m_arp = new Arp (this);
	m_arp_sender = new EthernetArpMacSender (this);
	m_arp->set_sender (m_arp_sender);
}

EthernetNetworkInterface::~EthernetNetworkInterface ()
{
	delete m_name;
	m_name = (std::string *)0xdeadbeaf;
	m_cable->unref ();
	m_cable = (Cable *)0xdeadbeaf;
	delete m_arp;
	m_arp = (Arp *)0xdeadbeaf;
	delete m_arp_sender;
	m_arp_sender = (EthernetArpMacSender *)0xdeadbeaf;
}

void 
EthernetNetworkInterface::set_host (Host *host)
{
	m_host = host;
	m_tracer = new NetworkInterfaceTracer (host, this);
}
NetworkInterfaceTracer *
EthernetNetworkInterface::get_tracer (void)
{
	return m_tracer;
}
void 
EthernetNetworkInterface::set_mac_address (MacAddress self)
{
	m_mac_address = self;
}
MacAddress 
EthernetNetworkInterface::get_mac_address (void)
{
	return m_mac_address;
}
std::string const *
EthernetNetworkInterface::get_name (void)
{
	return m_name;
}
uint16_t 
EthernetNetworkInterface::get_mtu (void)
{
	return 1000;
}
void 
EthernetNetworkInterface::set_up   (void)
{
	m_up = true;
}
void 
EthernetNetworkInterface::set_down (void)
{
	m_up = false;
}
bool 
EthernetNetworkInterface::is_down (void)
{
	return !m_up;
}
void 
EthernetNetworkInterface::set_ipv4_handler (Ipv4 *ipv4)
{
	m_ipv4 = ipv4;
}
void 
EthernetNetworkInterface::set_ipv4_address (Ipv4Address address)
{
	m_ipv4_address = address;
}
void 
EthernetNetworkInterface::set_ipv4_mask    (Ipv4Mask mask)
{
	m_ipv4_mask = mask;
}
Ipv4Address 
EthernetNetworkInterface::get_ipv4_address (void)
{
	return m_ipv4_address;
}
Ipv4Mask    
EthernetNetworkInterface::get_ipv4_mask    (void)
{
	return m_ipv4_mask;
}

void 
EthernetNetworkInterface::send (Packet *packet, Ipv4Address dest)
{
	m_arp->send_data (packet, dest);
}

void 
EthernetNetworkInterface::connect_to (Cable *cable)
{
	cable->ref ();
	m_cable = cable;
}
void 
EthernetNetworkInterface::recv (Packet *packet)
{
	m_tracer->trace_rx_mac (packet);
	ChunkMacLlcSnap *header;
	ChunkMacCrc *trailer;
	header = static_cast <ChunkMacLlcSnap *> (packet->remove_header ());
	trailer = static_cast <ChunkMacCrc *> (packet->remove_trailer ());
	switch (header->get_ether_type ()) {
	case ETHER_TYPE_ARP:
		m_arp->recv_arp (packet);
		break;
	case ETHER_TYPE_IPV4:
		m_ipv4->receive (packet, this);
		break;
	}
}

void 
EthernetNetworkInterface::send_data (Packet *packet, MacAddress dest)
{
	ChunkMacLlcSnap *header = new ChunkMacLlcSnap ();
	header->set_destination (dest);
	header->set_source (get_mac_address ());
	header->set_length (packet->get_size ());
	header->set_ether_type (ETHER_TYPE_IPV4);
	packet->add_header (header);
	ChunkMacCrc *trailer = new ChunkMacCrc ();
	packet->add_trailer (trailer);
	m_tracer->trace_tx_mac (packet);
	m_cable->send (packet, this);
}

void 
EthernetNetworkInterface::send_arp (Packet *packet, MacAddress dest)
{
	ChunkMacLlcSnap *header = new ChunkMacLlcSnap ();
	header->set_destination (dest);
	header->set_source (get_mac_address ());
	header->set_length (packet->get_size ());
	header->set_ether_type (ETHER_TYPE_ARP);
	packet->add_header (header);
	ChunkMacCrc *trailer = new ChunkMacCrc ();
	packet->add_trailer (trailer);
	m_tracer->trace_tx_mac (packet);
	m_cable->send (packet, this);
}
