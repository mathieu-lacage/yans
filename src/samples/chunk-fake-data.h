/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef CHUNK_FAKE_DATA_H
#define CHUNK_FAKE_DATA_H

#include "chunk.h"

class ChunkFakeData : public Chunk {
public:
	ChunkFakeData (uint32_t len);
	virtual ~ChunkFakeData ();

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	uint32_t m_len;
};

#endif /* CHUNK_FAKE_DATA_H */
