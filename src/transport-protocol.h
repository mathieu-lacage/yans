/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TRANSPORT_PROTOCOL_H
#define TRANSPORT_PROTOCOL_H

class Packet;

class TransportProtocol {
public:
	virtual ~TransportProtocol ();

	virtual uint8_t get_protocol (void) = 0;
	virtual void receive (Packet *packet) = 0;
};

#endif /* TRANSPORT_PROTOCOL_H */
