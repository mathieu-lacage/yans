/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef CHUNK_IPV4_H
#define CHUNK_IPV4_H

#include "chunk.h"

class ChunkIpv4 : public Chunk {
public:
	ChunkIpv4 ();
	virtual ~ChunkIpv4 ();

	void set_payload_size (uint16_t size);
	uint16_t get_payload_size (void);

	void set_tos (uint8_t);
	uint8_t get_tos (void);
	void set_id (uint16_t);
	uint16_t get_id (void);

	void set_more_fragments (void);
	void set_last_fragment (void);
	bool is_last_fragment (void);

	void set_fragment_offset (uint16_t offset);
	uint16_t get_fragment_offset (void);

	void set_ttl (uint8_t);
	uint8_t get_ttl (void);
	
	uint8_t get_protocol (void);
	void set_protocol (uint8_t);

	void set_source (uint32_t src);
	uint32_t get_source (void);

	void set_destination (uint32_t dst);
	uint32_t get_destination (void);

	bool is_checksum_ok (void);

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);
private:
	uint16_t calc_checksum (void);

	uint8_t m_ver_ihl;
	uint8_t m_tos;
	uint16_t m_id;
	uint16_t m_total_length;
	uint16_t m_fragment_offset;
	uint8_t m_ttl;
	uint8_t m_protocol;
	uint16_t m_checksum;
	uint32_t m_source;
	uint32_t m_destination;
};


#endif /* CHUNK_IPV4_H */
