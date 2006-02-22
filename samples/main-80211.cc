/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "network-interface-80211.h"
#include "network-interface-80211-factory.h"
#include "channel-80211.h"
#include "ipv4-route.h"
#include "simulator.h"
#include "udp-source.h"
#include "udp-sink.h"
#include "periodic-generator.h"
#include "traffic-analyser.h"
#include "callback.tcc"
#include "pcap-writer.h"
#include "trace-container.h"

#include <iostream>

using namespace yans;

class MyTrace {
public:
	void total_rx_bytes (uint64_t prev, uint64_t now) {
		std::cout << "at="<<Simulator::now_s ()
			  << ", rx=" << now - prev << std::endl;
	}
private:
};

static void
setup_rx_trace (NetworkInterface80211 *wifi)
{
	MyTrace *trace = new MyTrace ();
	TraceContainer *container = new TraceContainer ();
	wifi->register_trace (container);
	container->set_ui_variable_callback ("80211-bytes-rx", make_callback (&MyTrace::total_rx_bytes, trace));
}

int main (int argc, char *argv[])
{
	NetworkInterface80211Factory *wifi_factory;
	wifi_factory = new NetworkInterface80211Factory ();

	NetworkInterface80211 *wifi_client, *wifi_server;
	wifi_client = wifi_factory->create ();
	wifi_server = wifi_factory->create ();
	wifi_client->set_mac_address (MacAddress ("00:00:00:00:00:01"));
	wifi_server->set_mac_address (MacAddress ("00:00:00:00:00:02"));
	Channel80211 *channel = new Channel80211 ();
	wifi_client->connect_to (channel);
	wifi_server->connect_to (channel);

	setup_rx_trace (wifi_server);

	/* associate ipv4 addresses to the ethernet network elements */
	wifi_client->set_ipv4_address (Ipv4Address ("192.168.0.3"));
	wifi_client->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));
	wifi_server->set_ipv4_address (Ipv4Address ("192.168.0.2"));
	wifi_server->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));
	wifi_client->set_up ();
	wifi_server->set_up ();

	/* create hosts for the network elements*/
	Host *hclient, *hserver;
	hclient = new Host ("client");
	hserver = new Host ("server");
	hclient->add_interface (wifi_client);
	hserver->add_interface (wifi_server);

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  wifi_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  wifi_server);

	/* create udp source endpoint. */
	UdpSource *source = new UdpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->set_peer (Ipv4Address ("192.168.0.2"), 1026);
	source->unbind_at (11.0);
	/* create udp sink endpoint. */
	UdpSink *sink = new UdpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->unbind_at (11.0);


	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.01);
	generator->set_packet_size (100);
	generator->start_at (1.0);
	generator->stop_at (500.0);
	generator->set_send_callback (make_callback (&UdpSource::send, source));

	TrafficAnalyser *analyser = new TrafficAnalyser ();
	sink->set_receive_callback (make_callback (&TrafficAnalyser::receive, analyser));

	/* run simulation */
	Simulator::run ();

	analyser->print_stats ();


	/* destroy network */
	delete wifi_client;
	delete wifi_server;
	delete wifi_factory;
	delete channel;
	delete source;
	delete generator;
	delete sink;
	delete analyser;
	delete hclient;
	delete hserver;
	Simulator::destroy ();

	return 0;
}
