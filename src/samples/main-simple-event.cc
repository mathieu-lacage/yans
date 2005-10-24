/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "host.h"
#include "ethernet-network-interface.h"
#include "ipv4-route.h"
#include "cable.h"
#include "simulator.h"
#include "udp-source.h"
#include "udp-sink.h"

int main (int argc, char *argv[])
{
	/* setup the ethernet network itself. */
	EthernetNetworkInterface *eth_client, *eth_server;
	eth_client = new EthernetNetworkInterface ();
	eth_server = new EthernetNetworkInterface ();
	eth_client->set_mac_address (MacAddress ("00:00:00:00:00:01"));
	eth_server->set_mac_address (MacAddress ("00:00:00:00:00:02"));
	Cable *cable = new Cable ();
	cable->connect_to (eth_client);
	cable->connect_to (eth_server);
	cable->unref ();

	/* associate ipv4 addresses to the ethernet network elements */
	eth_client->set_ipv4_address (Ipv4Address ("192.168.0.3"));
	eth_client->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));
	eth_server->set_ipv4_address (Ipv4Address ("192.168.0.2"));
	eth_server->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));
	eth_client->set_up ();
	eth_server->set_up ();

	/* create hosts for the network elements*/
	Host *hclient, *hserver;
	hclient = new Host ("client");
	hserver = new Host ("server");
	hclient->add_interface (eth_client);
	hserver->add_interface (eth_server);

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  eth_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  eth_server);

	/* start applications. */
	UdpSource *source = new UdpSource (hclient);
	source->bind ();
	UdpSink *sink = new UdpSink (hserver);
	sink->bind ();

	source->set_peer (sink->get_address (), sink->get_port ());
	source->set_packet_interval (0.01);
	source->set_packet_length (100);
	source->start_at (1.0);


	/* run simulation */
	Simulator::instance ()->run ();


	/* destroy network */
	delete hclient;
	delete hserver;
	Simulator::instance ()->destroy ();

	return 0;
}
