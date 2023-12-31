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
#include <fstream>

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


class Interface80211Logger {
public:
	Interface80211Logger (std::ostream *os, std::string line);
	void register_traces (TraceContainer *container);
	void register_dca_traces (TraceContainer *container);
private:
	void notify_state (uint64_t start, uint64_t duration, uint8_t state);
	void notify_end_sync (bool rx_status);
	void notify_start_rx (uint64_t duration_us, double energy_w);
	void notify_start_sync (uint64_t duration_us, double energy_w);
	void notify_start_tx (uint64_t duration_us, uint32_t tx_mode, double tx_power);
	void notify_cw (uint64_t ov, uint64_t nv);
	void notify_backoff (uint64_t duration_us);
	void notify_acktimeout (uint32_t slrc);
	void notify_ctstimeout (uint32_t ssrc);
private:
	std::ostream *m_os;
	std::string m_line;
};

Interface80211Logger::Interface80211Logger (std::ostream *os, std::string line)
	: m_os (os),
	  m_line (line)
{}

void 
Interface80211Logger::register_traces (TraceContainer *container)
{
	container->set_callback ("80211-sync-end",
				 make_callback (&Interface80211Logger::notify_end_sync, this));
	container->set_callback ("80211-rx-start",
				 make_callback (&Interface80211Logger::notify_start_rx, this));
	container->set_callback ("80211-sync-start",
				 make_callback (&Interface80211Logger::notify_start_sync, this));
	container->set_callback ("80211-tx-start",
				 make_callback (&Interface80211Logger::notify_start_tx, this));
	container->set_callback ("80211-phy-state",
				 make_callback (&Interface80211Logger::notify_state, this));
}
void
Interface80211Logger::notify_cw (uint64_t old_cw, uint64_t new_cw)
{
	(*m_os) << "event-int "<<m_line<<" cw "<<new_cw<<" "<<Simulator::now_us ()<<std::endl;
}
void
Interface80211Logger::notify_backoff (uint64_t duration_us)
{
	(*m_os) << "event-int "<<m_line<<" bk "<<duration_us<<" "<<Simulator::now_us ()<<std::endl;
}
void 
Interface80211Logger::notify_acktimeout (uint32_t slrc)
{
	(*m_os) << "event-int "<<m_line<<" acktimeout "<<slrc<< " "<<Simulator::now_us ()<<std::endl;
}
void 
Interface80211Logger::notify_ctstimeout (uint32_t ssrc)
{
	(*m_os) << "event-int "<<m_line<<" ctstimeout "<<ssrc<< " "<<Simulator::now_us ()<<std::endl;
}
void 
Interface80211Logger::register_dca_traces (TraceContainer *container)
{
	container->set_ui_variable_callback ("80211-dcf-cw", 
					     make_callback (&Interface80211Logger::notify_cw, this));
	container->set_callback ("80211-dcf-backoff",
				 make_callback (&Interface80211Logger::notify_backoff, this));
	container->set_callback ("80211-dca-acktimeout",
				 make_callback (&Interface80211Logger::notify_acktimeout, this));
	container->set_callback ("80211-dca-ctstimeout",
				 make_callback (&Interface80211Logger::notify_ctstimeout, this));
}
void 
Interface80211Logger::notify_state (uint64_t start, uint64_t duration, uint8_t state)
{
	switch (state) {
	case 0:
		(*m_os) << "range "<<m_line<<" phy-state tx "<<start<<" "<<start+duration<<std::endl;
		break;
	case 1:
		(*m_os) << "range "<<m_line<<" phy-state sync "<<start<<" "<<start+duration<<std::endl;
		break;
	case 2:
		(*m_os) << "range "<<m_line<<" phy-state cca-busy "<<start<<" "<<start+duration<<std::endl;
		break;
	case 3:
		(*m_os) << "range "<<m_line<<" phy-state idle "<<start<<" "<<start+duration<<std::endl;
		break;
	}
}
void 
Interface80211Logger::notify_end_sync (bool rx_status)
{
	uint64_t now = Simulator::now_us ();
	std::string status;
	if (rx_status) {
		status = "ok";
	} else {
		status = "fail";
	}
	(*m_os) << "event-str "<<m_line<<" phy-rx " << status <<" "<<now<<std::endl;
}
void 
Interface80211Logger::notify_start_rx (uint64_t duration_us, double energy_w)
{
	// record event
}
void 
Interface80211Logger::notify_start_sync (uint64_t duration_us, double energy_w)
{}
void 
Interface80211Logger::notify_start_tx (uint64_t duration_us, uint32_t tx_mode, double tx_power)
{
	uint64_t now = Simulator::now_us ();
	(*m_os) << "event-int "<<m_line<<" tx " << (tx_mode/1000000)<<" "<<now<<std::endl;
	//(*m_os) << "event "<<m_line<<" tx-power " << tx_power<<std::endl;
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


	NetworkInterface80211Nqap *wifi_ap;
	StaticPosition *pos_ap;
	pos_ap = new StaticPosition ();
	wifi_ap = wifi_factory->create_nqap(address.get_next(), pos_ap);
	wifi_ap->connect_to(channel);
	wifi_ap->set_ssid("Hi");
	pos_ap->set(0.0,0.0,0.0);
	Host *hap = new Host ("ap");
	uint32_t ni_ap =
		hap->add_ipv4_arp_interface (wifi_ap,
					     Ipv4Address ("192.168.0.1"), 
					     Ipv4Mask ("255.255.255.0"));
	hap->get_routing_table ()->add_network_route_to(Ipv4Address ("192.168.0.0"),
							Ipv4Mask ("255.255.0.0"),
							ni_ap);


	// Create Station 1
	NetworkInterface80211Nqsta *wifi_client;
	StaticPosition *pos_client;
	pos_client = new StaticPosition ();
	wifi_client = wifi_factory->create_nqsta (address.get_next (), pos_client);
	wifi_client->connect_to (channel);
	wifi_client->start_active_association ("Hi");
	pos_client->set (5.0, 0.0, 0.0);
	Simulator::schedule_rel_s (1.0, make_event (&advance, pos_client));
	Host *hclient = new Host ("client");
	uint32_t ni_client =
		hclient->add_ipv4_arp_interface (wifi_client, 
						 Ipv4Address ("192.168.0.3"), 
						 Ipv4Mask ("255.255.255.0"));
	hclient->get_routing_table ()->add_network_route_to(Ipv4Address ("192.168.0.0"),
							    Ipv4Mask ("255.255.0.0"),
							    ni_client);
	UdpSource *source = new UdpSource (hclient);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->set_peer (Ipv4Address ("192.168.0.2"), 1026);
	source->unbind_at (10000.0);
	PeriodicGenerator *generator = new PeriodicGenerator ();
	generator->set_packet_interval (0.00001);
	generator->set_packet_size (2000);
	Simulator::schedule_rel_us (1000000,
				    make_event (&PeriodicGenerator::start_now, generator));
	generator->stop_at (42.0);
	generator->set_send_callback (make_callback (&UdpSource::send, source));


	// Create Station 2
	NetworkInterface80211Nqsta *wifi_server;
	StaticPosition *pos_server = new StaticPosition ();
	wifi_server = wifi_factory->create_nqsta (address.get_next (), pos_server);
	wifi_server->connect_to (channel);
	wifi_server->start_active_association ("Hi");
	pos_server->set (10.0,10.0, 10.0);
	ThroughputPrinter *printer = new ThroughputPrinter ();
	Simulator::schedule_abs_s (40, make_event (&ThroughputPrinter::stop, printer));
	printer->stop ();
	TraceContainer container = TraceContainer ();
	wifi_server->register_traces (&container);
	container.set_packet_logger_callback ("80211-packet-rx", 
					      make_callback (&ThroughputPrinter::receive, 
							     printer));
	Host *hserver = new Host ("server");
	uint32_t ni_server =
		hserver->add_ipv4_arp_interface (wifi_server, 
						 Ipv4Address ("192.168.0.2"), 
						 Ipv4Mask ("255.255.255.0"));
	hserver->get_routing_table ()->add_network_route_to(Ipv4Address ("192.168.0.0"),
							    Ipv4Mask ("255.255.0.0"),
							    ni_server);
	UdpSink *sink = new UdpSink (hserver);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->unbind_at (10000.0);

	std::ofstream my_output;
	my_output.open ("ap-phy.trace");
	TraceContainer tracer;
	//tracer.print_debug ();
	Interface80211Logger logger_ap = Interface80211Logger (&my_output, "ap");
	wifi_ap->register_traces (&tracer);
	logger_ap.register_traces (&tracer);
	wifi_ap->register_dca_traces (&tracer);
	logger_ap.register_dca_traces (&tracer);

	Interface80211Logger logger_sta1 = Interface80211Logger (&my_output, "sta1");
	wifi_client->register_traces (&tracer);
	logger_sta1.register_traces (&tracer);
	wifi_client->register_dca_traces (&tracer);
	logger_sta1.register_dca_traces (&tracer);

	Interface80211Logger logger_sta2 = Interface80211Logger (&my_output, "sta2");
	wifi_server->register_traces (&tracer);
	logger_sta2.register_traces (&tracer);
	wifi_server->register_dca_traces (&tracer);
	logger_sta2.register_dca_traces (&tracer);


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
