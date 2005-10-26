/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef NETWORK_INTERFACE_TRACER_H
#define NETWORK_INTERFACE_TRACER_H

#include <stdint.h>

class Host;
class Packet;
class WriteFile;
class WriteBuffer;
class NetworkInterface;

class NetworkInterfaceTracer {
public:
	NetworkInterfaceTracer (Host *host, NetworkInterface *interface);
	~NetworkInterfaceTracer ();

	void trace_tx_mac (Packet *packet);
	void trace_rx_mac (Packet *packet);

	void enable_all (void);
	void disable_all (void);
private:
	void write_buffer_to_file (void);
	void write_pcap_header (void);
	void write_packet_header (uint32_t size);
	void write_32 (uint32_t data);
	void write_16 (uint16_t data);

	Host *m_host;
	NetworkInterface *m_interface;
	WriteFile *m_file;
	WriteBuffer *m_buffer;
	bool m_enable_all;
};

#endif /* NETWORK_INTERFACE_TRACER_H */
