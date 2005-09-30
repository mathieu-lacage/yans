/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "chunk-udp.h"
#include "buffer.h"

/* The magic values below are used only for debugging.
 * They can be used to easily detect memory corruption
 * problems so you can see the patterns in memory.
 */
ChunkUdp::ChunkUdp ()
	: m_source_port (0xfffd),
	  m_destination_port (0xfffd),
	  m_udp_length (0xfffd)
{}
ChunkUdp::~ChunkUdp ()
{
	m_source_port = 0xfffe;
	m_destination_port = 0xffffe;
	m_udp_length = 0xfffe;
}

void 
ChunkUdp::set_destination (uint16_t port)
{
	m_destination_port = port;
}
void 
ChunkUdp::set_source (uint16_t port)
{
	m_source_port = port;
}
uint16_t 
ChunkUdp::get_source (void)
{
	return m_source_port;
}
uint16_t 
ChunkUdp::get_destination (void)
{
	return m_destination_port;
}
void 
ChunkUdp::set_payload_size (uint16_t size)
{
	m_udp_length = size + 8;
}
uint32_t 
ChunkUdp::get_size (void)
{
	return 8;
}
Chunk *
ChunkUdp::copy (void)
{
	ChunkUdp *udp_chunk = new ChunkUdp ();
	*udp_chunk = *this;
	return udp_chunk;
}
void 
ChunkUdp::serialize (WriteBuffer *buffer)
{
	buffer->write_htons_u16 (m_source_port);
	buffer->write_htons_u16 (m_destination_port);
	buffer->write_htons_u16 (m_udp_length);
	/* we do not calculate the udp header checksum
	 * This is bad but, well...
	 */
	buffer->write_htons_u16 (0);
}
void 
ChunkUdp::deserialize (ReadBuffer *buffer)
{
	m_source_port = buffer->read_nstoh_u16 ();
	m_destination_port = buffer->read_nstoh_u16 ();
	m_udp_length = buffer->read_nstoh_u16 ();
	uint16_t checksum = buffer->read_nstoh_u16 ();
	assert (checksum == 0);
}

void 
ChunkUdp::print (std::ostream *os)
{
	*os << "udp -- ";
	*os << " port source: " << m_source_port;
	*os << " post destination: " << m_destination_port;
	*os << " length: " << m_udp_length;
}


