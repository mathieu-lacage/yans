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
#include <boost/python.hpp>

void export_simulator (void);
void export_function_holder (void);
void export_thread (void);
void export_periodic_generator (void);
void export_traffic_analyser (void);
void export_host (void);
void export_udp_source (void);
void export_udp_sink (void);
void export_ipv4_address (void);
void export_mac_address (void);
void export_mac_network_interface (void);
void export_ipv4_network_interface (void);
void export_ethernet_network_interface (void);
void export_cable (void);
void export_ipv4_route (void);
void export_pcap_writer (void);
void export_trace_container (void);
void export_tcp_source (void);
void export_tcp_sink (void);
void export_cpp_callback_factory (void);


BOOST_PYTHON_MODULE(_yans)
{
	export_simulator ();
	export_function_holder ();
	export_thread ();
	export_cpp_callback_factory ();
	export_periodic_generator ();
	export_traffic_analyser ();
	export_host ();
	export_udp_source ();
	export_udp_sink ();
	export_ipv4_address ();
	export_mac_address ();
	export_mac_network_interface ();
	export_ipv4_network_interface ();
	export_ethernet_network_interface ();
	export_cable ();
	export_ipv4_route ();
	export_pcap_writer ();
	export_trace_container ();
	export_tcp_source ();
	export_tcp_sink ();
}
