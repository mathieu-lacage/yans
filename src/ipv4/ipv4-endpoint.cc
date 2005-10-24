/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ipv4-endpoint.h"

const uint16_t Ipv4EndPoints::PORT_RESERVED = 5000;



Ipv4EndPoints::Ipv4EndPoints ()
	: m_ephemeral (1024)
{}
Ipv4EndPoints::~Ipv4EndPoints ()
{
	m_ephemeral = 0x6666;
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		EndPoints *end_points = (*i).second;
		for (EndPointsI j = end_points->begin (); j != end_points->end (); j++) {
			delete (*j);
		}
		end_points->erase (end_points->begin (), end_points->end ());
	}
	m_addresses.erase (m_addresses.begin (), m_addresses.end ());
}

Ipv4EndPoint *
Ipv4EndPoints::allocate (Ipv4EndPointListener *listener)
{
	Ipv4EndPoint *end_point = new Ipv4EndPoint (listener, Ipv4Address::get_any (), 0);
	return end_point;
}
Ipv4EndPoint *
Ipv4EndPoints::allocate (Ipv4EndPointListener *listener,
			 Ipv4Address address)
{
	/* This is the allocation of an ephemeral port. */
	uint16_t port = m_ephemeral;
	do {
		port++;
		if (port > PORT_RESERVED) {
			port = PORT_RESERVED;
		}
		if (lookup (address, port) == 0) {
			Ipv4EndPoint *end_point = insert (listener, address, port);
			return end_point;
		}
	} while (port != m_ephemeral);

	return 0;
}
Ipv4EndPoint *
Ipv4EndPoints::allocate (Ipv4EndPointListener *listener,
			 Ipv4Address address, uint16_t port)
{
	if (lookup (address, port) != 0) {
		return 0;
	}
	Ipv4EndPoint *end_point = insert (listener, address, port);
	return end_point;
}

Ipv4EndPoint *
Ipv4EndPoints::insert (Ipv4EndPointListener *listener, 
		       Ipv4Address address, uint16_t port)
{
	
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		if ((*i).first.is_equal (address)) {
			Ipv4EndPoint *end_point = new Ipv4EndPoint (listener, address, port);
			(*i).second->push_back (end_point);
			return end_point;
		}
	}
	m_addresses.push_back (make_pair (address, new EndPoints ()));
	Ipv4EndPoint *end_point = insert (listener, address, port);
	return end_point;
}

Ipv4EndPoint *
Ipv4EndPoints::lookup (Ipv4Address address, uint16_t port)
{
	for (AddressesI i = m_addresses.begin (); i != m_addresses.end (); i++) {
		if ((*i).first.is_equal (address)) {
			EndPoints *end_points = (*i).second;
			for (EndPointsI j = end_points->begin (); j != end_points->end (); j++) {
				if ((*j)->get_port () == port) {
					return (*j);
				}
			}
		}
	}
	return 0;
}




uint16_t 
Ipv4EndPoint::get_port (void)
{
	return m_port;
}
Ipv4Address 
Ipv4EndPoint::get_address (void)
{
	return m_address;
}
Ipv4EndPointListener *
Ipv4EndPoint::get_listener (void)
{
	return m_listener;
}
Ipv4EndPoint::Ipv4EndPoint (Ipv4EndPointListener *listener,
			    Ipv4Address address,
			    uint16_t port)
	: m_listener (listener),
	  m_address (address),
	  m_port (port)
{}

Ipv4EndPoint::~Ipv4EndPoint ()
{
	m_address = Ipv4Address::get_broadcast ();
	m_port = 0x6666;
	m_listener = (Ipv4EndPointListener *)0xdeadbeaf;
}


Ipv4EndPointListener::~Ipv4EndPointListener ()
{}
