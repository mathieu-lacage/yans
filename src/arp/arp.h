/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef ARP_H
#define ARP_H

#include "mac-address.h"
#include "ipv4-address.h"
#include "sgi-hashmap.h"

class Packet;
class NetworkInterface;
class ArpCacheEntry;

class ArpMacSender {
public:
	virtual ~ArpMacSender () = 0;
	virtual void send (Packet *packet, MacAddress dest) = 0;
};

class Arp {
 public:
	Arp (NetworkInterface *interface);
	~Arp ();

	void set_alive_timeout (double aliveTimeout);
	void set_dead_timeout (double deadTimeout);
	void set_wait_reply_timeout (double waitReplyTimeout);
	double get_alive_timeout (void);
	double get_dead_timeout (void);
	double get_wait_reply_timeout (void);

	void set_sender (ArpMacSender *sender);

	/* send a packet through the ArpMacSender callback. */
	void send_data (Packet *packet, Ipv4Address to);
	/* receive an ARP packet. */
	void recv_arp (Packet *packet);
private:
	typedef Sgi::hash_map<Ipv4Address, ArpCacheEntry *, Ipv4AddressHash> ArpCache;
	typedef Sgi::hash_map<Ipv4Address, ArpCacheEntry *, Ipv4AddressHash>::iterator ArpCacheI;

	void send_arp_reply (Ipv4Address to_ip, MacAddress to_mac);
	void send_arp_request (Ipv4Address to);
	void drop_dead_packet (Packet *packet);

	NetworkInterface *m_interface;
	ArpMacSender *m_sender;
	double m_alive_timeout;
	double m_dead_timeout;
	double m_wait_reply_timeout;
	ArpCache m_arp_cache;
};

#endif /* ARP_H */
