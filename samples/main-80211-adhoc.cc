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

#include "yans/host.h"
#include "yans/network-interface-80211.h"
#include "yans/network-interface-80211-factory.h"
#include "yans/channel-80211.h"
#include "yans/ipv4-route.h"
#include "yans/simulator.h"
#include "yans/udp-source.h"
#include "yans/udp-sink.h"
#include "yans/periodic-generator.h"
#include "yans/traffic-analyser.h"
#include "yans/callback.h"
#include "yans/pcap-writer.h"
#include "yans/trace-container.h"
#include "yans/event.tcc"
#include "yans/static-position.h"
#include "yans/mac-address-factory.h"
#include "yans/throughput-printer.h"

#include <iostream>

using namespace yans;

static void 
advance (StaticPosition *a) 
{
	double x,y,z;
	a->get (x,y,z);
	x += 5.0;
	a->set (x,y,z);
	if (x >= 210.0) {
		return;
	}
	//std::cout << "x="<<x << std::endl;
	Simulator::schedule_rel_s (1.0, make_event (&advance, a));
}


int main (int argc, char *argv[])
{
	Simulator::set_linked_list ();

	//Simulator::enable_log_to ("80211.log");

	NetworkInterface80211Factory *wifi_factory;
	wifi_factory = new NetworkInterface80211Factory ();
	// force rts/cts on all the time.
	wifi_factory->set_mac_rts_cts_threshold (2200);
	wifi_factory->set_mac_fragmentation_threshold (2200);
	//wifi_factory->set_cr (5, 5);
	//wifi_factory->set_ideal (1e-5);
	wifi_factory->set_aarf ();
	//wifi_factory->set_arf ();

	Channel80211 *channel = new Channel80211 ();
	MacAddressFactory address;


	NetworkInterface80211Adhoc *wifi_client;
	StaticPosition *pos_client;
	pos_client = new StaticPosition ();
	wifi_client = wifi_factory->create_adhoc (address.get_next (), pos_client);
	wifi_client->connect_to (channel);
	pos_client->set (5.0, 0.0, 0.0);
	Simulator::schedule_rel_s (1.0, make_event (&advance, pos_client));
	Host *hclient = new Host ("client");
	Ipv4NetworkInterface *ni_client =
		hclient->add_ipv4_arp_interface (wifi_client, 
						 Ipv4Address ("192.168.0.3"), 
						 Ipv4Mask ("255.255.255.0"));
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  ni_client);
	UdpSource *source = new UdpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->set_peer (Ipv4Address ("192.168.0.2"), 1026);
	source->unbind_at (10000.0);
	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.00001);
	generator->set_packet_size (2000);
	generator->start_now ();
	generator->stop_at (42.0);
	generator->set_send_callback (make_callback (&UdpSource::send, source));



	NetworkInterface80211Adhoc *wifi_server;
	StaticPosition *pos_server = new StaticPosition ();
	wifi_server = wifi_factory->create_adhoc (address.get_next (), pos_server);
	wifi_server->connect_to (channel);
	pos_server->set (0.0, 0.0, 0.0);
	ThroughputPrinter *printer = new ThroughputPrinter ();
	Simulator::schedule_abs_s (40, make_event (&ThroughputPrinter::stop, printer));
	TraceContainer container = TraceContainer ();
	wifi_server->register_trace (&container);
	container.set_packet_logger_callback ("80211-packet-rx", 
					      make_callback (&ThroughputPrinter::receive, printer));
	Host *hserver = new Host ("server");

	Ipv4NetworkInterface *ni_server =
		hserver->add_ipv4_arp_interface (wifi_server, 
						 Ipv4Address ("192.168.0.2"), 
						 Ipv4Mask ("255.255.255.0"));
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  ni_server);
	UdpSink *sink = new UdpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->unbind_at (10000.0);


	/* run simulation */
	Simulator::run ();

	/* destroy network */
	delete wifi_client;
	delete wifi_server;
	delete wifi_factory;
	delete channel;
	delete source;
	delete generator;
	delete sink;
	delete printer;
	delete hclient;
	delete hserver;
	Simulator::destroy ();

	return 0;
}
