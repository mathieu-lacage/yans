/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "chunk-tcp.h"
#include "utils.h"

ChunkTcp::ChunkTcp ()
	: m_source_port (0),
	  m_destination_port (0),
	  m_sequence_number (0),
	  m_ack_number (0),
	  m_header_length (5),
	  m_flags (0),
	  m_window_size (0),
	  m_checksum (0),
	  m_urgent_pointer (0),
	  m_mss (0)
{}

ChunkTcp::~ChunkTcp ()
{}

void 
ChunkTcp::set_source_port (uint16_t port)
{
	m_source_port = htons (port);
}
void 
ChunkTcp::set_destination_port (uint16_t port)
{
	m_destination_port = htons (port);
}
void 
ChunkTcp::set_sequence_number (uint32_t sequence)
{
	m_sequence_number = htons (sequence);
}
void ChunkTcp::set_window_size (uint16_t size)
{
	m_window_size = htons (size);
}

uint16_t 
ChunkTcp::get_source_port (void)
{
	return nstoh (m_source_port);
}
uint16_t 
ChunkTcp::get_destination_port (void)
{
	return nstoh (m_destination_port);
}
uint32_t 
ChunkTcp::get_sequence_number (void)
{
	return nstoh (m_sequence_number);
}
uint32_t 
ChunkTcp::get_ack_number (void)
{
	return nstoh (m_ack_number);
}
uint16_t 
ChunkTcp::get_window_size (void)
{
	return nstoh (m_window_size);
}
uint16_t 
ChunkTcp::get_urgent_pointer (void)
{
	return nstoh (m_urgent_pointer);
}
uint16_t 
ChunkTcp::get_option_mss (void)
{
	return (m_mss >> 16) & 0xffff;
}
void 
ChunkTcp::enable_flag_syn (void)
{
	enable_flag (SYN);
}
void 
ChunkTcp::enable_flag_fin (void)
{
	enable_flag (FIN);
}
void 
ChunkTcp::enable_flag_rst (void)
{
	enable_flag (RST);
}
void 
ChunkTcp::enable_flag_ack (uint32_t ack)
{
	enable_flag (ACK);
	m_ack_number = htons (ack);
}
void 
ChunkTcp::enable_flag_urg (uint16_t ptr)
{
	enable_flag (URG);
	m_urgent_pointer = htons (ptr);
}
void 
ChunkTcp::enable_flag_psh (void)
{
	enable_flag (PSH);
}
void 
ChunkTcp::enable_option_mss (uint16_t mss)
{
	uint32_t real = 0;
	real = (2) | (4 << 8) | (mss << 16);
	m_mss = htons (real);
	m_header_length = 6;
}

void 
ChunkTcp::disable_flag_syn (void)
{
	disable_flag (SYN);
}
void 
ChunkTcp::disable_flag_fin (void)
{
	disable_flag (FIN);
}
void 
ChunkTcp::disable_flag_rst (void)
{
	disable_flag (RST);
}
void 
ChunkTcp::disable_flag_ack (void)
{
	disable_flag (ACK);
}
void 
ChunkTcp::disable_flag_urg (void)
{
	disable_flag (URG);
}
void 
ChunkTcp::disable_flag_psh (void)
{
	disable_flag (PSH);
}

bool 
ChunkTcp::is_flag_syn (void)
{
	return is_flag (SYN);
}
bool 
ChunkTcp::is_flag_fin (void)
{
	return is_flag (FIN);
}
bool 
ChunkTcp::is_flag_rst (void)
{
	return is_flag (RST);
}
bool 
ChunkTcp::is_flag_ack (void)
{
	return is_flag (ACK);
}
bool 
ChunkTcp::is_flag_urg (void)
{
	return is_flag (URG);
}
bool 
ChunkTcp::is_flag_psh (void)
{
	return is_flag (PSH);
}
bool 
ChunkTcp::is_option_mss (void)
{
	assert ((m_mss != 0)?m_header_length == 6:m_header_length == 5);
	return (m_mss != 0)?true:false;
}


uint32_t 
ChunkTcp::get_size (void)
{
	uint32_t size = 0;
	size += 20;
	if (is_option_mss ()) {
		size += 4;
	}
	return size;
}

Chunk *
ChunkTcp::copy (void)
{
	ChunkTcp *chunk = new ChunkTcp ();
	*chunk = *this;
	return chunk;
}

void
ChunkTcp::serialize (WriteBuffer *buffer)
{}

void
ChunkTcp::deserialize (ReadBuffer *buffer)
{}

void
ChunkTcp::print (std::ostream *os)
{}


bool 
ChunkTcp::is_flag (uint8_t n)
{
	if (m_flags & (1 << n)) {
		return true;
	} else {
		return false;
	}
}
void 
ChunkTcp::enable_flag (uint8_t n)
{
	m_flags |= 1 << n;
}
void 
ChunkTcp::disable_flag (uint8_t n)
{
	m_flags &= ~(1 << n);
}
