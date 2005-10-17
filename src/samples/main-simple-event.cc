/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "host.h"
#include "ethernet-network-interface.h"
#include "cable.h"
#include "simulator.h"

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
	


	/* run simulation */
	Simulator::instance ()->run ();


	/* destroy network */
	delete hclient;
	delete hserver;
	Simulator::instance ()->destroy ();

	return 0;
}
