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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

class SimpleClient : public ProcessLoop {
public:
	SimpleClient ()
private:
	virtual int run (int argc, char *argv[]);
};

SimpleClient::SimpleClient ()
{}
int
SimpleClient::run (int argc, char *argv[])
{
	NetworkInterface *interface = get_host ()->lookup_interface ("eth0");
	interface->set_ipv4_address (Ipv4Address ("192.168.0.3"));
	interface->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));

	SocketUdp *socket = get_host ()->create_socket_udp ();
	socket->set_peer (Ipv4Address ("192.168.0.2"), 80);
	printf ("send fake data %d\n", 500);
	socket->send (new ChunkFakeData (500));

	return 0;
}



class SimpleServer : public ProcessLoop {
public:
	SimpleServer ();
private:
	virtual int run (int argc, char *argv[]);
};

SimpleServer::SimpleServer ()
{}
int
SimpleServer::run (int argc, char *argv[])
{
	NetworkInterface *interface = get_host ()->lookup_interface ("eth0");
	interface->set_ipv4_address (Ipv4Address ("192.168.0.2"));
	interface->set_ipv4_mask (Ipv4Mask ("255.255.255.0"));

	SocketUdp *socket = get_host ()->create_socket_udp ();
	socket->bind (Ipv4Address::get_zero (), 80);
	ChunkFakeData *chunk = static_cast <ChunkFakeData *> (socket->recv ());
	printf ("received fake data %d\n", chunk->get_size ());

	return 0;
}


int
main (int argc, char *argv[])
{
	/* create the physical network topology. */
	EthernetNetworkInterface *lclient = new EthernetNetworkInterface ();
	lclient->set_mac_address (MacAddress ("00:00:00:00:00:01"));
	EthernetNetworkInterface *lserver = new EthernetNetworkInterface ();
	lserver->set_mac_address (MacAddress ("00:00:00:00:00:02"));
	EthernetCable *cable = new EthernetCable ();
	cable->connect_to (lclient);
	cable->connect_to (lserver);

	/* create server host, connect it to network. */
	Host *hserver = new Host ("server");
	new SimpleServer (hserver);
	hserver->add_interface (lserver);

	/* create client host, connect it to network. */
	Host *hclient = new Host ("client");
	new SimpleClient (hclient);
	hclient->add_interface (lclient);

	Simulator::instance ()->run ();

	return 0;
}
