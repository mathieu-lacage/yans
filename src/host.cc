/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "host.h"
#include "network-interface.h"
#include "ipv4.h"
#include "ipv4-route.h"

Host::Host ()
{
	m_ipv4 = new Ipv4 ();
	m_routing_table = new Ipv4Route ();
}

Host::~Host ()
{
	delete m_ipv4;
	delete m_routing_table;
	for (NetworkInterfacesI i = m_interfaces.begin (); 
	     i != m_interfaces.end (); 
	     i = m_interfaces.erase (i)) {
		delete (*i);
	}
}

Ipv4Route *
Host::get_routing_table (void)
{
	return m_routing_table;
}

NetworkInterfaces const *
Host::get_interfaces (void)
{
	return &m_interfaces;
}

NetworkInterface *
Host::lookup_interface (char const *name)
{
	for (NetworkInterfacesCI i = m_interfaces.begin ();
	     i != m_interfaces.end ();
	     i++) {
		if ((*i)->get_name ()->compare (name) == 0) {
			return (*i);
		}
	}
	return 0;
}
void 
Host::add_interface (NetworkInterface *interface)
{
	m_interfaces.push_back (interface);
}
