/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <ostream>
#include <vector>

class Chunk;
class WriteBuffer;
class ReadBuffer;

class Packet {
public:
	Packet ();
	~Packet ();

	void add_header (Chunk *header);
	void add_trailer (Chunk *trailer);
	Chunk *remove_header (void);
	Chunk *remove_trailer (void);
	Chunk const *peek_header (void);
	Chunk const *peek_trailer (void);

	uint32_t get_size (void);

	void serialize (WriteBuffer *buffer);

	void print (std::ostream *os);

	Packet *copy (void);
 private:
	typedef std::vector<Chunk *> Headers;
	typedef std::vector<Chunk *> Trailers;
	typedef std::vector<Chunk *>::const_iterator HeadersCI;
	typedef std::vector<Chunk *>::reverse_iterator HeadersRI;
	typedef std::vector<Chunk *>::const_iterator TrailersCI;
	/* The last header added is pushed at the front 
	 * of the header list while the last trailer added
	 * is pushed at the back of the trailer list.
	 */
	Headers m_headers;
	Trailers m_trailers;
};

#endif /* PACKET_H */
