/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

class Buffer {
public:
	void write_u8 (uint8_t data);
	uint8_t read_u8 (void);

	void write_u16 (uint16_t data);
	uint16_t read_u16 (void);

	void write_u32 (uint32_t data);
	uint32_t read_u32 (void);

	void write_htons_u32 (uint32_t data);
	uint32_t read_nstoh_u32 (void);
};

#endif /* BUFFER_H */
