/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CHUNK_MAC_LLC_SNAP_H
#define CHUNK_MAC_LLC_SNAP_H

#include "chunk.h"
#include "mac-address.h"
#include "ipv4-address.h"

class ChunkMacLlcSnap : public Chunk {
 public:
	ChunkMacLlcSnap ();
	virtual ~ChunkMacLlcSnap ();

	void set_source (MacAddress source);
	void set_destination (MacAddress source);
	void set_length (uint16_t length);
	uint16_t get_length (void);
	MacAddress get_source (void);
	MacAddress get_destination (void);

	void set_ether_type (uint16_t ether_type);
	uint16_t get_ether_type (void);
	

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	MacAddress m_source;
	MacAddress m_destination;
	uint16_t m_length;
	uint16_t m_ether_type;
};

#endif /* CHUNK_MAC_LLC_SNAP_H */
