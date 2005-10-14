/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CHUNK_MAC_CRC_H
#define CHUNK_MAC_CRC_H

#include "chunk.h"
#include "mac-address.h"
#include "ipv4-address.h"

class ChunkMacCrc : public Chunk {
 public:
	ChunkMacCrc ();
	virtual ~ChunkMacCrc ();	

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
};

#endif /* CHUNK_MAC_LLC_SNAP_H */
