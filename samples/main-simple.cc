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
#include "periodic-generator.h"
#include "traffic-analyser.h"
#include "callback.tcc"
#include "pcap-writer.h"
#include "trace-container.h"

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

	TraceContainer *client_trace = new TraceContainer ();
	TraceContainer *server_trace = new TraceContainer ();
	eth_client->register_trace(client_trace);
	eth_server->register_trace (server_trace);


	PcapWriter *client_pcap = new PcapWriter ();
	PcapWriter *server_pcap = new PcapWriter ();
	client_pcap->open ("client-log-eth0");
	server_pcap->open ("server-log-eth0");
	client_trace->set_packet_logger_callback ("ethernet-send", 
						  make_callback (&PcapWriter::write_packet,
								 client_pcap));
	client_trace->set_packet_logger_callback ("ethernet-recv", 
						  make_callback (&PcapWriter::write_packet,
								 client_pcap));
	server_trace->set_packet_logger_callback ("ethernet-send", 
						  make_callback (&PcapWriter::write_packet,
								 server_pcap));
	server_trace->set_packet_logger_callback ("ethernet-recv", 
						  make_callback (&PcapWriter::write_packet,
								 server_pcap));

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  eth_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  eth_server);

	/* create udp source endpoint. */
	UdpSource *source = new UdpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->set_peer (Ipv4Address ("192.168.0.2"), 1026);
	source->unbind_at (110.0);
	/* create udp sink endpoint. */
	UdpSink *sink = new UdpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->unbind_at (110.0);


	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.00001);
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
	delete eth_client;
	delete eth_server;
	delete cable;
	delete source;
	delete generator;
	delete sink;
	delete analyser;
	delete hclient;
	delete hserver;
	delete client_pcap;
	delete server_pcap;
	delete client_trace;
	delete server_trace;
	Simulator::destroy ();

	return 0;
}
