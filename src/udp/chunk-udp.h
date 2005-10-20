/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef CHUNK_UDP_H
#define CHUNK_UDP_H

#include <stdint.h>
#include "udp.h"
#include "chunk.h"

class ChunkUdp : public Chunk {
public:
	ChunkUdp ();
	virtual ~ChunkUdp ();

	void set_destination (uint16_t port);
	void set_source (uint16_t port);
	uint16_t get_source (void);
	uint16_t get_destination (void);

	void set_payload_size (uint16_t size);

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	uint16_t m_source_port;
	uint16_t m_destination_port;
	uint16_t m_udp_length;
};

#endif /* CHUNK_UDP */
