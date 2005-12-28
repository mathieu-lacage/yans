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
#include "tcp-source.h"
#include "tcp-sink.h"
#include "host-tracer.h"
#include "network-interface-tracer.h"
#include "periodic-generator.h"
#include "traffic-analyzer.h"
#include "event.h"

using namespace yans;

int main (int argc, char *argv[])
{
	/* setup the ethernet network itself. */
	EthernetNetworkInterface *eth_client, *eth_server;
	eth_client = new EthernetNetworkInterface ("eth0");
	eth_server = new EthernetNetworkInterface ("eth0");
	eth_client->set_mac_address (MacAddress ("00:00:00:00:00:01"));
	eth_server->set_mac_address (MacAddress ("00:00:00:00:00:02"));
	Cable *cable = new Cable ();
	cable->connect_to (eth_client, eth_server);
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
	//hclient->get_tracer ()->enable_all ();
	//hserver->get_tracer ()->enable_all ();
	hclient->add_interface (eth_client);
	hserver->add_interface (eth_server);
	eth_client->get_tracer ()->enable_all ();
	eth_server->get_tracer ()->enable_all ();

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  eth_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  eth_server);

	TcpSource *source = new TcpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->start_connect_at (Ipv4Address ("192.168.0.2"), 1026, 1.0);
	source->start_disconnect_at (11.0);

	TcpSink *sink = new TcpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->stop_listen_at (12.0);

	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.1);
	generator->set_packet_size (100);
	generator->start_at (2.0);
	generator->stop_at (10.0);
	generator->set_send_callback (make_callback (&TcpSource::send, source));

	TrafficAnalyzer *analyzer = new TrafficAnalyzer ();
	sink->set_receive_callback (make_callback (&TrafficAnalyzer::receive, analyzer));


	/* run simulation */
	Simulator::run ();

	analyzer->print_stats ();


	/* destroy network */
	delete source;
	delete generator;
	delete sink;
	delete analyzer;
	delete hclient;
	delete hserver;
	Simulator::destroy ();

	return 0;
}
