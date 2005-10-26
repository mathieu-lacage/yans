/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_ENDPOINT_H
#define IPV4_ENDPOINT_H

#include <stdint.h>
#include "ipv4-address.h"
#include <list>

class Packet;

class Ipv4EndPointListener {
public:
	virtual ~Ipv4EndPointListener () = 0;
	virtual void receive (Packet *packet) = 0;
};

class Ipv4EndPoint {
public:
	~Ipv4EndPoint ();
	uint16_t get_port (void);
	Ipv4Address get_address (void);
	Ipv4EndPointListener *get_listener (void);
private:
	friend class Ipv4EndPoints;
	Ipv4EndPoint (Ipv4EndPointListener *listener,
		      Ipv4Address address,
		      uint16_t port);
	Ipv4EndPointListener *m_listener;
	Ipv4Address m_address;
	uint16_t m_port;
};

class Ipv4EndPoints {
public:
	Ipv4EndPoints ();
	~Ipv4EndPoints ();

	Ipv4EndPoint *allocate (Ipv4EndPointListener *listener);
	Ipv4EndPoint *allocate (Ipv4EndPointListener *listener,
				Ipv4Address address);
	Ipv4EndPoint *allocate (Ipv4EndPointListener *listener,
				Ipv4Address address, uint16_t port);
	void destroy (Ipv4EndPoint *end_point);

	Ipv4EndPoint *lookup (Ipv4Address address, uint16_t port);
private:
	Ipv4EndPoint *insert (Ipv4EndPointListener *listener,
			      Ipv4Address address, uint16_t port);

	static const uint16_t PORT_RESERVED;

	typedef std::list<Ipv4EndPoint *> EndPoints;
	typedef std::list<Ipv4EndPoint *>::iterator EndPointsI;
	typedef std::list<std::pair<Ipv4Address, EndPoints *> > Addresses;
	typedef std::list<std::pair<Ipv4Address, EndPoints *> >::iterator AddressesI;
	Addresses m_addresses;
	uint16_t m_ephemeral;
};


#endif /* IPV4_ENDPOINT_H */
