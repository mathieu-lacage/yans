/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef HOST_TRACER_H
#define HOST_TRACER_H

class Host;
class Packet;
class WriteFile;

class HostTracer {
public:
	void trace_tx_app  (Packet *packet);
	void trace_tx_udp  (Packet *packet);
	void trace_tx_ipv4 (Packet *packet);

	void trace_rx_app  (Packet *packet);
	void trace_rx_udp  (Packet *packet);
	void trace_rx_ipv4 (Packet *packet);

	void enable_all (void);
	void disable_all (void);
private:
	friend class Host;
	HostTracer (WriteFile *file);
	~HostTracer ();

	bool m_enable_all;
	bool m_enable_app;
	bool m_enable_udp;
	bool m_enable_ipv4;
	WriteFile *m_file;
};

#endif /* HOST_TRACER_H */
