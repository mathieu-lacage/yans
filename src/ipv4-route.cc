/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "ipv4-route.h"
#include "ipv4.h"

/*****************************************************
 *           Host Route
 *****************************************************/

HostRoute::HostRoute (Ipv4Address dest,
		      Ipv4Address gateway,
		      NetworkInterface *interface)
	: m_dest (dest),
	  m_gateway (gateway),
	  m_interface (interface)
{}
HostRoute::HostRoute (Ipv4Address dest,
		      NetworkInterface *interface)
	: m_dest (dest),
	  m_gateway (Ipv4Address::get_zero ()),
	  m_interface (interface)
{}

Ipv4Address 
HostRoute::get_dest (void)
{
	return m_dest;
}
bool 
HostRoute::is_gateway (void)
{
	if (m_gateway.is_equal (Ipv4Address::get_zero ())) {
		return false;
	} else {
		return true;
	}
}
Ipv4Address 
HostRoute::get_gateway (void)
{
	return m_gateway;
}
NetworkInterface *
HostRoute::get_interface (void)
{
	return m_interface;
}


/*****************************************************
 *           Network Route
 *****************************************************/

NetworkRoute::NetworkRoute (Ipv4Address network,
			    Ipv4Mask network_mask,
			    Ipv4Address gateway,
			    NetworkInterface *interface)
	: m_network (network),
	  m_network_mask (network_mask),
	  m_gateway (gateway),
	  m_interface (interface)
{}
NetworkRoute::NetworkRoute (Ipv4Address network,
			    Ipv4Mask network_mask,
			    NetworkInterface *interface)
	: m_network (network),
	  m_network_mask (network_mask),
	  m_gateway (Ipv4Address::get_zero ()),
	  m_interface (interface)
{}

Ipv4Address 
NetworkRoute::get_dest_network (void)
{
	return m_network;
}
Ipv4Mask 
NetworkRoute::get_dest_network_mask (void)
{
	return m_network_mask;
}
bool 
NetworkRoute::is_gateway (void)
{
	if (m_gateway.is_equal (Ipv4Address::get_zero ())) {
		return false;
	} else {
		return true;
	}
}
Ipv4Address 
NetworkRoute::get_gateway (void)
{
	return m_gateway;
}
NetworkInterface *
NetworkRoute::get_interface (void)
{
	return m_interface;
}

/*****************************************************
 *           Default Route
 *****************************************************/

DefaultRoute::DefaultRoute (Ipv4Address gateway,
			    NetworkInterface *interface)
	: m_gateway (gateway),
	  m_interface (interface)
{}
Ipv4Address 
DefaultRoute::get_gateway (void)
{
	return m_gateway;
}
NetworkInterface *
DefaultRoute::get_interface (void)
{
	return m_interface;
}


/*****************************************************
 *           Very simple Ipv4 Routing module
 *****************************************************/

Ipv4Route::Ipv4Route ()
{}
Ipv4Route::~Ipv4Route ()
{}


void 
Ipv4Route::add_route_to (Ipv4Address dest, 
		    Ipv4Address next_hop, 
		    NetworkInterface *interface)
{}
void 
Ipv4Route::add_route_to (Ipv4Address dest, 
		    NetworkInterface *interface)
{}
void 
Ipv4Route::add_route_to (Ipv4Address network, 
		    Ipv4Mask network_mask, 
		    Ipv4Address next_hop, 
		    NetworkInterface *interface)
{}
void 
Ipv4Route::add_route_to (Ipv4Address network, 
		    Ipv4Mask network_mask, 
		    NetworkInterface *interface)
{}
void 
Ipv4Route::set_default_route (Ipv4Address next_hop, 
			 NetworkInterface *interface)
{}
HostRoute *
Ipv4Route::lookup_host (Ipv4Address dest)
{
	return 0;
}
NetworkRoute *
Ipv4Route::lookup_network (Ipv4Address dest)
{
	return 0;
}
DefaultRoute *
Ipv4Route::lookup_default (void)
{
	return 0;
}

