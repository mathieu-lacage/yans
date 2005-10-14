/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <ostream>

class WriteBuffer;
class ReadBuffer;

class Chunk {
public:
	virtual ~Chunk () = 0;

	virtual uint32_t get_size (void) = 0;
	virtual Chunk *copy (void) = 0;
	virtual void serialize (WriteBuffer *buffer) = 0;
	virtual void deserialize (ReadBuffer *buffer) = 0;
	virtual void print (std::ostream *os) = 0;
};

#endif /* CHUNK_H */
