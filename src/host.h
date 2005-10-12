/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef HOST_H
#define HOST_H

#include <vector>
#include "ipv4-address.h"

class Ipv4;
class Ipv4Route;
class NetworkInterface;
class SocketUdp;
class ReadFile;
class WriteFile;
class Process;
class Udp;

typedef std::vector<NetworkInterface *> NetworkInterfaces;
typedef std::vector<NetworkInterface *>::const_iterator NetworkInterfacesCI;

class Host {
public:
	Host (char const *path);
	~Host ();

	Ipv4Route *get_routing_table (void);

	NetworkInterfaces const *get_interfaces (void);
	NetworkInterface *lookup_interface (char const *name);
	void add_interface (NetworkInterface *interface);

	SocketUdp *create_socket_udp (void);
	ReadFile *open_for_read (char const *file);
	WriteFile *open_for_write (char const *file);
	Process *create_process (void);

private:
	typedef std::vector<NetworkInterface *>::iterator NetworkInterfacesI;
	NetworkInterfaces m_interfaces;
	Ipv4Route *m_routing_table;
	Ipv4 *m_ipv4;
	Udp *m_udp;
	std::string *m_root;
};


#endif /* HOST_H */
