/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_ROUTE_H
#define IPV4_ROUTE_H

#include <list>

#include "ipv4-address.h"

class NetworkInterface;

class Route {
public:
	Route (Route const &route);

	bool is_host (void) const;
	Ipv4Address get_dest (void) const;

	bool is_network (void) const;
	Ipv4Address get_dest_network (void) const;
	Ipv4Mask get_dest_network_mask (void) const;

	bool is_default (void) const;

	bool is_gateway (void) const;
	Ipv4Address get_gateway (void) const;

	NetworkInterface *get_interface (void) const;
private:
	friend class Ipv4Route;
	Route (Ipv4Address network,
	       Ipv4Mask mask,
	       Ipv4Address gateway,
	       NetworkInterface *interface);
	Route (Ipv4Address dest,
	       Ipv4Mask mask,
	       NetworkInterface *interface);
	Route (Ipv4Address dest,
	       Ipv4Address gateway,
	       NetworkInterface *interface);
	Route (Ipv4Address dest,
	       NetworkInterface *interface);

	Ipv4Address m_dest;
	Ipv4Mask m_dest_network_mask;
	Ipv4Address m_gateway;
	NetworkInterface *m_interface;
};

class Ipv4Route {
public:
	Ipv4Route ();
	~Ipv4Route ();

	/* add route to host dest through host next_hop 
	 * on interface.
	 */
	void add_host_route_to (Ipv4Address dest, 
				Ipv4Address next_hop, 
				NetworkInterface *interface);
	/* add route to host dest on interface.
	 */
	void add_host_route_to (Ipv4Address dest, 
				NetworkInterface *interface);
	/* add route to network dest with netmask 
	 * through host next_hop on interface
	 */
	void add_network_route_to (Ipv4Address network, 
				   Ipv4Mask network_mask, 
				   Ipv4Address next_hop, 
				   NetworkInterface *interface);
	/* add route to network dest with netmask 
	 * on interface
	 */
	void add_network_route_to (Ipv4Address network, 
				   Ipv4Mask network_mask, 
				   NetworkInterface *interface);
	/* set the default route to host next_hop on
	 * interface. 
	 */
	void set_default_route (Ipv4Address next_hop, 
				NetworkInterface *interface);

	Route *lookup (Ipv4Address dest);
private:
	typedef std::list<Route *> HostRoutes;
	typedef std::list<Route *>::const_iterator HostRoutesCI;
	typedef std::list<Route *>::iterator HostRoutesI;
	typedef std::list<Route *> NetworkRoutes;
	typedef std::list<Route *>::const_iterator NetworkRoutesCI;
	typedef std::list<Route *>::iterator NetworkRoutesI;

	HostRoutes m_host_routes;
	NetworkRoutes m_network_routes;
	Route *m_default_route;
};

#endif /* IPV4_ROUTE_H */
