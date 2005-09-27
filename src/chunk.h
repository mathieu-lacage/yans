#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

class Buffer;
class std::ostream;

class Chunk {
public:
	virtual ~Chunk ();

	virtual uint32_t get_size (void) = 0;
	virtual Chunk *copy (void) = 0;
	virtual void serialize (Buffer *buffer) = 0;
	virtual void deserialize (Buffer *buffer) = 0;
	virtual void print (std::ostream *os) = 0;
};

#endif /* CHUNK_H */
