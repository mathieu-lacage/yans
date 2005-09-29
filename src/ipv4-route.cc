void 
Ipv4::add_route_to (Ipv4Address dest, 
		    Ipv4Address next_hop, 
		    NetworkInterface *interface)
{}
void 
Ipv4::add_route_to (Ipv4Address dest, 
		    NetworkInterface *interface)
{}
void 
Ipv4::add_route_to (Ipv4Address network, 
		    Ipv4Mask network_mask, 
		    Ipv4Address next_hop, 
		    NetworkInterface *interface)
{}
void 
Ipv4::add_route_to (Ipv4Address network, 
		    Ipv4Mask network_mask, 
		    NetworkInterface *interface)
{}
void 
Ipv4::set_default_route (Ipv4Address next_hop, 
			 NetworkInterface *interface)
{}
HostEntry *
Ipv4::lookup_host (Ipv4Address dest)
{}
NetworkEntry *
Ipv4::lookup_network (Ipv4Address dest)
{}
DefaultEntry *
Ipv4::lookup_default (void)
{}

