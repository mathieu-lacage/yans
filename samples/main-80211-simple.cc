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
#include "yans/network-interface-80211-simple.h"
#include "yans/network-interface-80211-simple-factory.h"
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
#include "yans/mac-address-factory.h"
#include "yans/static-position.h"

#include <iostream>

using namespace yans;

class MyTrace {
public:
	MyTrace () {
		m_period_s = 1.0;
		m_current = 0;
		m_prev = 0;
	}
	void total_rx_bytes (uint64_t prev, uint64_t now) {
		m_current = now;
	}
	void start (PeriodicGenerator *generator, StaticPosition *a) {
		generator->start_now ();
		Simulator::insert_in_s (m_period_s, make_event (&MyTrace::advance, this, generator, a));
	}
	void advance (PeriodicGenerator *generator, StaticPosition *a) {
		generator->stop_now ();
		double x,y,z;
		a->get (x,y,z);
		x += 5.0;
		a->set (x,y,z);
		uint32_t n_bytes = m_current - m_prev;
		m_prev = m_current;
		double mbs = ((n_bytes * 8.0) /(1000000.0 *m_period_s));
		std::cout << "x="<<x<<", throughput="<<mbs<<"Mb/s"<<std::endl;
		if (x >= 250.0) {
			return;
		}
		generator->start_now ();
		Simulator::insert_in_s (m_period_s, make_event (&MyTrace::advance, this, generator, a));
	}
private:
	uint32_t m_current;
	uint32_t m_prev;
	double m_period_s;
};

static MyTrace *
setup_rx_trace (NetworkInterface80211Simple *wifi)
{
	MyTrace *trace = new MyTrace ();
	TraceContainer container = TraceContainer ();
	wifi->register_trace (&container);
	container.set_ui_variable_callback ("80211-bytes-rx", make_callback (&MyTrace::total_rx_bytes, trace));
	return trace;
}

int main (int argc, char *argv[])
{
	Simulator::set_binary_heap ();

	MacAddressFactory address;

	StaticPosition *pos_client, *pos_server;
	pos_client = new StaticPosition ();
	pos_server = new StaticPosition ();
	pos_client->set (0,0,0);
	pos_server->set (0,0,0);

	NetworkInterface80211SimpleFactory *wifi_factory;
	wifi_factory = new NetworkInterface80211SimpleFactory ();
	// force rts/cts on all the time.
	wifi_factory->set_rts_cts_threshold (1);
	//wifi_factory->set_cr (6, 6);
	//wifi_factory->set_ideal (1e-5);
	wifi_factory->set_aarf ();

	NetworkInterface80211Simple *wifi_client, *wifi_server;
	wifi_client = wifi_factory->create (address.get_next (), pos_client);
	wifi_server = wifi_factory->create (address.get_next (), pos_server);
	Channel80211 *channel = new Channel80211 ();
	wifi_client->connect_to (channel);
	wifi_server->connect_to (channel);

	MyTrace *trace = setup_rx_trace (wifi_server);

	/* create hosts for the network elements*/
	Host *hclient, *hserver;
	hclient = new Host ("client");
	hserver = new Host ("server");
	Ipv4NetworkInterface *ip_client, *ip_server;
	ip_client = hclient->add_ipv4_arp_interface (wifi_client, 
						     Ipv4Address ("192.168.0.3"),
						     Ipv4Mask ("255.255.255.0"));
	ip_server = hserver->add_ipv4_arp_interface (wifi_server,
						     Ipv4Address ("192.168.0.3"),
						     Ipv4Mask ("255.255.255.0"));

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  ip_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  ip_server);

	/* create udp source endpoint. */
	UdpSource *source = new UdpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->set_peer (Ipv4Address ("192.168.0.2"), 1026);
	source->unbind_at (10000.0);
	/* create udp sink endpoint. */
	UdpSink *sink = new UdpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->unbind_at (10000.0);

	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.00001);
	generator->set_packet_size (2000);
	trace->start (generator, pos_server);
	generator->set_send_callback (make_callback (&UdpSource::send, source));

	TrafficAnalyser *analyser = new TrafficAnalyser ();
	sink->set_receive_callback (make_callback (&TrafficAnalyser::receive, analyser));

	/* run simulation */
	Simulator::run ();

	//analyser->print_stats ();


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
	delete trace;
	Simulator::destroy ();

	return 0;
}
