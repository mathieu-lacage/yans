/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>


class WriteBuffer {
public:
	WriteBuffer (uint32_t prefered_size);
	~WriteBuffer ();

	/* used to copy the data out of this write buffer. */
	uint8_t *peek_data (void);
	uint32_t get_written_size (void);
	void reset (void);

	void write_u8 (uint8_t data);
	void write (uint8_t const*buffer, uint8_t size);
	void write_htons_u16 (uint16_t data);
	void write_htons_u32 (uint32_t data);

private:
	void ensure_write_room_left (uint8_t needed);
	uint32_t m_size;
	uint32_t m_write;
	uint8_t *m_buffer;
};

class ReadBuffer {
public:
	ReadBuffer (uint32_t size);
	~ReadBuffer ();

	/* used to copy the data in this read buffer. */
	uint8_t *peek_data (void);

	uint8_t read_u8 (void);
	void read (uint8_t *buffer, uint8_t size);
	uint16_t read_nstoh_u16 (void);
	uint32_t read_nstoh_u32 (void);

private:
	bool is_read_room_left (uint8_t needed);
	uint32_t m_size;
	uint32_t m_read;
	uint8_t *m_buffer;
};

#include "test.h"
class BufferTest: public Test {
public:
	BufferTest (TestManager *manager);
	
	virtual bool run_tests (void);
private:
	bool ensure_written_bytes (WriteBuffer *buffer, 
				   uint32_t n, uint8_t array[]);
};


#endif /* BUFFER_H */
