/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

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
	Host *hserver = new Host (new SimpleServer (), "server");
	hserver->add_interface (lserver);

	/* create client host, connect it to network. */
	Host *hclient = new Host (new SimpleClient (), "client");
	hclient->add_interface (lclient);

	Simulator::instance ()->run ();

	return 0;
}
