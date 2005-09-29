/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_ROUTE_H
#define IPV4_ROUTE_H

class HostRoute {
public:
	HostRoute (Ipv4Address dest,
		   Ipv4Address gateway,
		   NetworkInterface *interface);
	HostRoute (Ipv4Address dest,
		   NetworkInterface *interface);

	Ipv4Address get_dest (void);
	bool is_gateway (void);
	Ipv4Address get_gateway (void);
	NetworkInterface *get_interface (void);
private:
	Ipv4Address m_dest;
	Ipv4Address m_gateway;
	NetworkInterface *m_interface;
};
class NetworkRoute {
public:
	NetworkRoute (Ipv4Address network,
		      Ipv4Mask mask,
		      Ipv4Address gateway,
		      NetworkInterface *interface);
	NetworkRoute (Ipv4Address dest,
		      Ipv4Mask mask,
		      NetworkInterface *interface);
	Ipv4Address get_dest_network (void);
	Ipv4Mask get_dest_network_mask (void);
	bool is_gateway (void);
	Ipv4Address get_gateway (void);
	NetworkInterface *get_interface (void);
private:
	Ipv4Address m_network;
	Ipv4Mask m_network_mask;
	Ipv4Address m_gateway;
	NetworkInterface *m_interface;
};
class DefaultRoute {
public:
	DefaultRoute (Ipv4Address gateway,
		      NetworkInterface *interface);
	Ipv4Address get_gateway (void);
	NetworkInterface *get_interface (void);
private:
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
	void add_route_to (Ipv4Address dest, 
			   Ipv4Address next_hop, 
			   NetworkInterface *interface);
	/* add route to host dest on interface.
	 */
	void add_route_to (Ipv4Address dest, 
			   NetworkInterface *interface);
	/* add route to network dest with netmask 
	 * through host next_hop on interface
	 */
	void add_route_to (Ipv4Address network, 
			   Ipv4Mask network_mask, 
			   Ipv4Address next_hop, 
			   NetworkInterface *interface);
	/* add route to network dest with netmask 
	 * on interface
	 */
	void add_route_to (Ipv4Address network, 
			   Ipv4Mask network_mask, 
			   NetworkInterface *interface);
	/* set the default route to host next_hop on
	 * interface. 
	 */
	void set_default_route (Ipv4Address next_hop, 
				NetworkInterface *interface);

	HostRoute *lookup_host (Ipv4Address dest);
	NetworkRoute *lookup_network (Ipv4Address dest);
	DefaultRoute *lookup_default (void);

private:
	DefaultRoute *m_default_route;
};

#endif /* IPV4_ROUTE_H */
