/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "host.h"
#include "ethernet-network-interface.h"
#include "ipv4-route.h"
#include "cable.h"
#include "simulator.h"
#include "udp-source.h"
#include "udp-sink.h"
#include "host-tracer.h"
#include "network-interface-tracer.h"
#include "periodic-generator.h"
#include "traffic-analyser.h"

using namespace yans;

int main (int argc, char *argv[])
{
	/* setup the ethernet network itself. */
	EthernetNetworkInterface *eth_client, *eth_server, *eth_router_client, *eth_router_server;
	eth_client = new EthernetNetworkInterface ("eth0");
	eth_server = new EthernetNetworkInterface ("eth0");
	eth_router_client = new EthernetNetworkInterface ("eth0");
	eth_router_server = new EthernetNetworkInterface ("eth1");
	eth_client->set_mac_address (MacAddress ("00:00:00:00:00:01"));
	eth_server->set_mac_address (MacAddress ("00:00:00:00:00:02"));
	eth_router_client->set_mac_address (MacAddress ("00:00:00:00:00:03"));
	eth_router_server->set_mac_address (MacAddress ("00:00:00:00:00:04"));
	Cable *client_cable = new Cable ();
	client_cable->connect_to (eth_client, eth_router_client);
	Cable *server_cable = new Cable ();
	server_cable->connect_to (eth_server, eth_router_server);

	/* associate ipv4 addresses to the ethernet network elements */
	eth_client->set_ipv4_address (Ipv4Address ("192.168.0.2"));
	eth_client->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));
	eth_router_client->set_ipv4_address (Ipv4Address ("192.168.0.1"));
	eth_router_client->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));

	eth_server->set_ipv4_address (Ipv4Address ("192.168.1.2"));
	eth_server->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));
	eth_router_server->set_ipv4_address (Ipv4Address ("192.168.1.1"));
	eth_router_server->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));

	eth_client->set_up ();
	eth_server->set_up ();
	eth_router_client->set_up ();
	eth_router_server->set_up ();

	/* create hosts for the network elements*/
	Host *hclient, *hserver, *hrouter;
	hclient = new Host ("client");
	hserver = new Host ("server");
	hrouter = new Host ("router");
	eth_client->set_mtu (980);
	eth_router_server->set_mtu (979);

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.1"),
							  eth_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.1.1"),
							  eth_server);
	hrouter->get_routing_table ()->add_network_route_to (Ipv4Address ("192.168.0.0"),
							     Ipv4Mask ("255.255.255.0"),
							     eth_router_client);
	hrouter->get_routing_table ()->add_network_route_to (Ipv4Address ("192.168.1.0"),
							     Ipv4Mask ("255.255.255.0"),
							     eth_router_server);
	

	/* create udp source endpoint. */
	UdpSource *source = new UdpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.2"), 1025);
	source->set_peer (Ipv4Address ("192.168.1.2"), 1026);
	source->unbind_at (11.0);
	/* create udp sink endpoint. */
	UdpSink *sink = new UdpSink (hserver);
	sink->bind (Ipv4Address ("192.168.1.2"), 1026);
	sink->unbind_at (11.0);


	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.01);
	generator->set_packet_size (981);
	generator->start_at (1.0);
	generator->stop_at (10.0);
	generator->set_send_callback (make_callback (&UdpSource::send, source));

	TrafficAnalyser *analyser = new TrafficAnalyser ();
	sink->set_receive_callback (make_callback (&TrafficAnalyser::receive, analyser));

	/* run simulation */
	Simulator::run ();

	analyser->print_stats ();


	/* destroy network */
	delete eth_client;
	delete eth_server;
	delete client_cable;
	delete server_cable;
	delete source;
	delete generator;
	delete sink;
	delete analyser;
	delete hclient;
	delete hserver;
	delete hrouter;
	Simulator::destroy ();

	return 0;
}
