#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

class Chunk;
class Buffer;
class std::ostream;


class Packet {
public:
	Packet ();

	void append_header (Chunk *header);
	void prepend_trailer (Chunk *trailer);
	Chunk *remove_header (void);
	Chunk *remove_trailer (void);
	Chunk const *peek_header (void);
	Chunk const *peek_trailer (void);

	uint32_t get_size (void);

	void serialize (Buffer *buffer);
	void deserialize (Buffer const *buffer);

	void print (std::ostream *os);

	Packet *copy (void);
};

#endif /* PACKET_H */
