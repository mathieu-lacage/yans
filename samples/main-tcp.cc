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

#include "yans/host.h"
#include "yans/ethernet-network-interface.h"
#include "yans/ipv4-route.h"
#include "yans/cable.h"
#include "yans/simulator.h"
#include "yans/tcp-source.h"
#include "yans/tcp-sink.h"
#include "yans/periodic-generator.h"
#include "yans/traffic-analyser.h"
#include "yans/event.h"
#include "yans/trace-container.h"
#include "yans/callback-event.tcc"
#include <iostream>

using namespace yans;

class MyTcpTrace {
public:
	void start_connect (TcpSource *source, Ipv4Address address, uint16_t port) 
	{
		source->start_connect_now (address, port);

		TraceContainer *container = new TraceContainer ();
		source->register_trace (container);
		container->set_ui_variable_callback ("tcp-snd-nxt",
						     make_callback (&MyTcpTrace::notify_snd_nxt_change,
								    this));
		container->set_ui_variable_callback ("tcp-snd-wnd",
						     make_callback (&MyTcpTrace::notify_snd_wnd_change,
								    this));
		container->set_ui_variable_callback ("tcp-snd-cwnd",
						     make_callback (&MyTcpTrace::notify_snd_cwnd_change,
								    this));
		container->set_ui_variable_callback ("tcp-snd-ssthresh",
						     make_callback (&MyTcpTrace::notify_snd_ssthresh_change,
								    this));
		delete container;
	}
	void notify_snd_nxt_change (uint64_t old_val, uint64_t new_val)
	{
		std::cout << "snd_nxt="<<old_val<<"->"<<new_val<<std::endl;
	}
	void notify_snd_wnd_change (uint64_t old_val, uint64_t new_val)
	{
		std::cout << "snd_wnd="<<old_val<<"->"<<new_val<<std::endl;
	}
	void notify_snd_cwnd_change (uint64_t old_val, uint64_t new_val)
	{
		std::cout << "snd_cwnd="<<old_val<<"->"<<new_val<<std::endl;
	}
	void notify_snd_ssthresh_change (uint64_t old_val, uint64_t new_val)
	{
		std::cout << "snd_ssthresh="<<old_val<<"->"<<new_val<<std::endl;
	}
};

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

	TcpSource *source = new TcpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->start_disconnect_at (11.0);

	/* setup tcp tracing at source 
	 * we must wait until we call start_connection on the source
	 * to connect the trace because the underlying connection
	 * object which we want to trace does not exist before.
	 */
	MyTcpTrace *tcp_trace = new MyTcpTrace ();
	Simulator::insert_at_s (1.0, make_event (&MyTcpTrace::start_connect, tcp_trace, 
						 source, Ipv4Address ("192.168.0.2"), (uint16_t)1026));

	TcpSink *sink = new TcpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->stop_listen_at (12.0);

	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.1);
	generator->set_packet_size (100);
	generator->start_at (2.0);
	generator->stop_at (10.0);
	generator->set_send_callback (make_callback (&TcpSource::send, source));

	TrafficAnalyser *analyser = new TrafficAnalyser ();
	sink->set_receive_callback (make_callback (&TrafficAnalyser::receive, analyser));


	/* run simulation */
	Simulator::run ();

	analyser->print_stats ();


	/* destroy network */
	delete eth_client;
	delete eth_server;
	delete cable;
	delete source;
	delete generator;
	delete sink;
	delete analyser;
	delete hclient;
	delete hserver;
	delete tcp_trace;
	Simulator::destroy ();

	return 0;
}
