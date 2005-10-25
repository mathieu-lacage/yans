/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H

#include "chunk.h"

class ChunkData : public Chunk {
public:
	ChunkData (uint8_t const *buffer, uint32_t len);
	virtual ~ChunkData ();

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	uint8_t *m_data;
	uint32_t m_len;
};

#endif /* CHUNK_DATA_H */
