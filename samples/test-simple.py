#!/usr/bin/env python

from yans import *

def setup_pcap_trace(interface, name):
    container = TraceContainer ()
    interface.register_trace (container)
    #container.print_debug ()
    writer = PcapWriter ()
    writer.open (name)
    writer.write_header_ethernet ()
    container.set_packet_logger_callback ("ethernet-send",
                                          make_callback (PcapWriter.write_packet,
                                                         writer))
    container.set_packet_logger_callback ("ethernet-recv",
                                          make_callback (PcapWriter.write_packet,
                                                         writer))
    del container
    return writer

client_address = Ipv4Address ('192.168.0.3')
server_address = Ipv4Address ('192.168.0.2')
netmask = Ipv4Mask ('255.255.255.0')

eth_client = EthernetNetworkInterface (MacAddress ('00:00:00:00:00:01'), 'eth0')
eth_server = EthernetNetworkInterface (MacAddress ('00:00:00:00:00:02'), 'eth0')

cable = Cable ()
cable.connect_to (eth_client, eth_server)

client_writer = setup_pcap_trace (eth_client, "client-eth0")
server_writer = setup_pcap_trace (eth_server, "server-eth0")

hclient = Host ('client')
hserver = Host ('server')
ip_client = hclient.add_ipv4_arp_interface (eth_client, client_address, netmask)
ip_server = hserver.add_ipv4_arp_interface (eth_server, server_address, netmask)

hclient.get_routing_table ().set_default_route (server_address, ip_client);
hserver.get_routing_table ().set_default_route (client_address, ip_server);

source = UdpSource (hclient)
source.bind (client_address, 1025)
source.set_peer (server_address, 1026)
source.unbind_at (110.0)

sink = UdpSink (hserver)
sink.bind (server_address, 1026)
sink.unbind_at (110.0)

generator = PeriodicGenerator ()
generator.set_packet_interval (0.01)
generator.set_packet_size (100)
generator.start_at (1.0)
generator.stop_at (100.0)
generator.set_send_callback (make_callback (UdpSource.send, source))

analyser = TrafficAnalyser ()
sink.set_receive_callback (make_callback (TrafficAnalyser.receive, analyser))

simulator.run ()

analyser.print_stats ()

del source
del sink
del generator
del analyser
del hclient
del hserver
del client_address
del server_address
del eth_client
del eth_server
del client_writer
del server_writer

simulator.destroy ()
