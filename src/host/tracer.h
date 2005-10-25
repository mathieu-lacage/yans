/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TRACER_H
#define TRACER_H

class Host;
class Packet;
class WriteFile;

class Tracer {
public:
	void trace_tx_app  (Packet *packet);
	void trace_tx_udp  (Packet *packet);
	void trace_tx_ipv4 (Packet *packet);
	void trace_tx_arp (Packet *packet);
	void trace_tx_mac (Packet *packet);
	void trace_tx_phy (Packet *packet);

	void trace_rx_app  (Packet *packet);
	void trace_rx_udp  (Packet *packet);
	void trace_rx_ipv4 (Packet *packet);
	void trace_rx_arp (Packet *packet);
	void trace_rx_mac (Packet *packet);
	void trace_rx_phy (Packet *packet);

	void enable_all (void);
	void disable_all (void);
private:
	friend class Host;
	Tracer (WriteFile *file);
	~Tracer ();

	bool m_enable_all;
	bool m_enable_app;
	bool m_enable_udp;
	bool m_enable_ipv4;
	WriteFile *m_file;
};

#endif /* TRACER_H */
