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
#include "yans/udp-source.h"
#include "yans/udp-sink.h"
#include "yans/periodic-generator.h"
#include "yans/traffic-analyser.h"
#include "yans/callback.h"
#include "yans/pcap-writer.h"
#include "yans/trace-container.h"

using namespace yans;

PcapWriter *
setup_pcap_trace (EthernetNetworkInterface *interface, char const *name)
{
	TraceContainer *trace = new TraceContainer ();
	interface->register_trace(trace);

	PcapWriter *pcap = new PcapWriter ();
	pcap->open (name);
	pcap->write_header_ethernet ();
	trace->set_packet_logger_callback ("ethernet-send", 
					   make_callback (&PcapWriter::write_packet,
							  pcap));
	trace->set_packet_logger_callback ("ethernet-recv", 
					   make_callback (&PcapWriter::write_packet,
							  pcap));
	delete trace;
	return pcap;
}

int main (int argc, char *argv[])
{
	/* setup the ethernet network itself. */
	EthernetNetworkInterface *eth_client, *eth_server;
	eth_client = new EthernetNetworkInterface (MacAddress ("00:00:00:00:00:01"), "eth0");
	eth_server = new EthernetNetworkInterface (MacAddress ("00:00:00:00:00:02"), "eth0");
	Cable *cable = new Cable ();
	cable->connect_to (eth_client, eth_server);

	/* create hosts for the network elements*/
	Host *hclient, *hserver;
	hclient = new Host ("client");
	hserver = new Host ("server");
	uint32_t ip_client, ip_server;
	ip_client = hclient->add_ipv4_arp_interface (eth_client, 
						     Ipv4Address ("192.168.0.3"),
						     Ipv4Mask ("255.255.255.0"));
	ip_server = hserver->add_ipv4_arp_interface (eth_server,
						     Ipv4Address ("192.168.0.2"),
						     Ipv4Mask ("255.255.255.0"));

	/* setup tracing for eth0-level interfaces in pcap files. */
	PcapWriter *client_pcap = setup_pcap_trace (eth_client, "client-log-eth0");
	PcapWriter *server_pcap = setup_pcap_trace (eth_server, "server-log-eth0");

	/* setup the routing tables. */
	hclient->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.2"),
							  ip_client);
	hserver->get_routing_table ()->set_default_route (Ipv4Address ("192.168.0.3"),
							  ip_server);

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
	Simulator::destroy ();

	return 0;
}
