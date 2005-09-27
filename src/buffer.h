/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

/***
 * This Buffer class is a boundless in-memory buffer of bytes. 
 * It will grow to accomodate the need of the class users.
 * It can be written to and read from only once until
 * the reset method is called again.
 * Typical client code looks like this:
 *
 * File *file = fopen ("packet.dump", "w");
 * Buffer *buffer = new Buffer ();
 * while (true) {
 *         Packet *packet = container.next ();
 *         packet->serialize (buffer);
 *         fwrite (file, packet->peek_data (), packet->get_written_size ());
 *         packet->reset ();
 * }
 */
class Buffer {
public:
	Buffer (uint32_t prefered_size);
	~Buffer ();

	void write_u8 (uint8_t data);
	uint8_t read_u8 (void);

	void write_u16 (uint16_t data);
	uint16_t read_u16 (void);

	void write_u32 (uint32_t data);
	uint32_t read_u32 (void);

	void write_htons_u32 (uint32_t data);
	uint32_t read_nstoh_u32 (void);

	void reset (void);
	uint32_t get_written_size (void);
	uint8_t *peek_data (void);
private:
	void ensure_write_room_left (uint8_t needed);
	bool is_read_room_left (uint8_t needed);
	uint32_t m_size;
	uint32_t m_write;
	uint32_t m_read;
	uint8_t *m_buffer;
};

#include "test.h"
class BufferTest: public Test {
public:
	BufferTest (TestManager *manager);
	
	virtual bool run_tests (void);
};


#endif /* BUFFER_H */
