/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "chunk-mac-crc.h"
#include "buffer.h"

ChunkMacCrc::ChunkMacCrc ()
{}
ChunkMacCrc::~ChunkMacCrc ()	
{}

uint32_t 
ChunkMacCrc::get_size (void)
{
	return 4;
}
Chunk *
ChunkMacCrc::copy (void)
{
	ChunkMacCrc *chunk = new ChunkMacCrc ();
	return chunk;
}
void 
ChunkMacCrc::serialize (WriteBuffer *buffer)
{
	buffer->write_u8 (0);
	buffer->write_u8 (0);
	buffer->write_u8 (0);
	buffer->write_u8 (0);
}
void 
ChunkMacCrc::deserialize (ReadBuffer *buffer)
{
	uint8_t crc[4];
	buffer->read (crc, 4);
	assert (crc[0] == 0);
	assert (crc[1] == 0);
	assert (crc[2] == 0);
	assert (crc[3] == 0);
}
void 
ChunkMacCrc::print (std::ostream *os)
{
	*os << "(mac) crc";
}
