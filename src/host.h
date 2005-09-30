/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef HOST_H
#define HOST_H

#include <vector>
#include "ipv4-address.h"

class Ipv4;
class Ipv4Route;
class NetworkInterface;

typedef std::vector<NetworkInterface *> NetworkInterfaces;
typedef std::vector<NetworkInterface *>::const_iterator NetworkInterfacesCI;

class Host {
public:
	Host ();
	~Host ();

	Ipv4Route *get_routing_table (void);

	NetworkInterfaces const *get_interfaces (void);

	NetworkInterface *lookup_interface (char const *name);
	void add_interface (NetworkInterface *interface);

private:
	typedef std::vector<NetworkInterface *>::iterator NetworkInterfacesI;
	NetworkInterfaces m_interfaces;
	Ipv4Route *m_routing_table;
	Ipv4 *m_ipv4;
};


#endif /* HOST_H */
