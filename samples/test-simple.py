#!/usr/bin/env python

from yans import *

client_mac_address = MacAddress ('00:00:00:00:00:01')
server_mac_address = MacAddress ('00:00:00:00:00:02')
client_address = Ipv4Address ('192.168.0.3')
server_address = Ipv4Address ('192.168.0.2')
netmask = Ipv4Mask ('255.255.255.0')

eth_client = EthernetNetworkInterface ('eth0')
eth_server = EthernetNetworkInterface ('eth0')

eth_client.set_mac_address (client_mac_address)
eth_server.set_mac_address (server_mac_address)

cable = Cable ()
cable.connect_to (eth_client, eth_server)

eth_client.set_ipv4_address (client_address)
eth_client.set_ipv4_mask (netmask)
eth_server.set_ipv4_address (server_address)
eth_server.set_ipv4_mask (netmask)
eth_client.set_up ()
eth_server.set_up ()


hclient = Host ('client')
hserver = Host ('server')
hclient.add_interface (eth_client)
hserver.add_interface (eth_server)

source = UdpSource (hclient)
source.bind (client_address, 1025)
source.set_peer (server_address, 1026)
source.unbind_at (11.0)

sink = UdpSink (hserver)
sink.bind (server_address, 1026)
sink.unbind_at (11.0)

generator = PeriodicGenerator ()
generator.set_packet_interval (0.01)
generator.set_packet_size (100)
generator.start_at (1.0)
generator.stop_at (10.0)
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
del client_mac_address
del server_mac_address
del eth_client
del eth_server

simulator.destroy ()
