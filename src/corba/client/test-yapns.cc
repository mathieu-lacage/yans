/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "yapns/host.h"
#include "yapns/simulation-context.h"
#include "yapns/simulator.h"
#include "yapns/mac-address-factory.h"
#include "yapns/network-interface-80211-factory.h"
#include "yapns/network-interface-80211.h"
#include "yapns/static-position.h"
#include "yapns/channel-80211.h"
#include "yapns/ipv4-route.h"
#include "yapns/udp-source.h"
#include "yapns/udp-sink.h"
#include "yapns/periodic-generator.h"
#include "yapns/traffic-analyser.h"
#include "yapns/callback.h"
#include "yapns/event.tcc"

using namespace yapns;

void 
advance (PeriodicGenerator *generator, StaticPosition *a) 
{
	double x,y,z;
	a->get (x,y,z);
	x += 5.0;
	a->set (x,y,z);
	if (x >= 210.0) {
		return;
	}
	Simulator::insert_in_s (1.0, make_event (&advance, generator, a));
}


int main (int argc, char *argv[])
{
	SimulationContextFactory ctx;
	ctx.read_configuration ("sample.xml");
	NetworkInterface80211Factory factory;
	MacAddressFactory address_factory;
	Channel80211 *channel = new Channel80211 ();

	SimulationContext a_context = ctx.lookup ("a");
	Host *a = new Host (a_context, "a");
	StaticPosition *pos_a = new StaticPosition (a_context);
	NetworkInterface80211Adhoc *interface_a = factory.create_adhoc (a_context, 
									address_factory.get_next (), 
									pos_a);
	interface_a->connect_to (channel);
	Ipv4NetworkInterface *ipv4_interface_a = a->add_ipv4_arp_interface (interface_a,
									    Ipv4Address ("192.168.0.1"),
									    Ipv4Mask ("255.255.255.0"));
	Ipv4Route *routing_table_a = a->get_routing_table ();
	routing_table_a->set_default_route (Ipv4Address ("192.168.0.2"), ipv4_interface_a);
	UdpSource *source = new UdpSource (a_context, a);
	source->bind (Ipv4Address ("192.168.0.3"), 1025);
	source->set_peer (Ipv4Address ("192.168.0.2"), 1026);
	source->unbind_at (10000.0);
	PeriodicGenerator *generator = new PeriodicGenerator (a_context);
	generator->set_packet_interval (0.00001);
	generator->set_packet_size (2000);
	generator->set_send_callback (make_callback (&UdpSource::send, source));
	double x, y, z;
	x = 5.0;
	y = 0.0;
	z = 0.0;
	pos_a->set (x,y,z);
	generator->start_now ();
	Simulator::insert_in_s (1.0, make_event (&advance, generator, pos_a));
	



	SimulationContext b_context = ctx.lookup ("b");
	Host *b = new Host (b_context, "b");
	Position *pos_b = new StaticPosition (b_context);
	NetworkInterface80211Adhoc *interface_b = factory.create_adhoc (b_context, 
									address_factory.get_next (), 
									pos_b);
	interface_b->connect_to (channel);
	Ipv4NetworkInterface *ipv4_interface_b = b->add_ipv4_arp_interface (interface_b,
									    Ipv4Address ("192.168.0.2"),
									    Ipv4Mask ("255.255.255.0"));
	Ipv4Route *routing_table_b = b->get_routing_table ();
	routing_table_b->set_default_route (Ipv4Address ("192.168.0.1"), ipv4_interface_b);
	UdpSink *sink = new UdpSink (b_context, b);
	sink->bind (Ipv4Address ("192.168.0.2"), 1026);
	sink->unbind_at (10000.0);
	TrafficAnalyser *analyser = new TrafficAnalyser (b_context);
	sink->set_receive_callback (make_callback (&TrafficAnalyser::receive, analyser));





	
	

	Simulator::run ();

	delete channel;

	delete a;
	delete pos_a;
	delete interface_a;
	delete source;
	delete generator;

	delete b;
	delete pos_b;
	delete interface_b;
	delete sink;
	delete analyser;

	Simulator::destroy ();

	return 0;
}
