/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "chunk-data.h"
#include "buffer.h"
#include <string.h>

ChunkData::ChunkData (uint8_t const *buffer, uint32_t len)
	: m_data (new uint8_t [len]),
	  m_len (len)
{
	memcpy (m_data, buffer, len);
}
ChunkData::~ChunkData ()
{
	delete [] m_data;
	m_data = (uint8_t *)0xdeadbeaf;
	m_len = 0x33;
}

uint32_t 
ChunkData::get_size (void)
{
	return m_len;
}
Chunk *
ChunkData::copy (void)
{
	ChunkData *other = new ChunkData (m_data, m_len);
	return other;
}
void 
ChunkData::serialize (WriteBuffer *buffer)
{
	buffer->write (m_data, m_len);
}
void 
ChunkData::deserialize (ReadBuffer *buffer)
{
	buffer->read (m_data, m_len);
}
void 
ChunkData::print (std::ostream *os)
{
	*os << "data -- ";
	*os << " len: " << m_len;
}
