/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "chunk.h"
#include "chunk-ipv4.h"
#include "utils.h"
#include "buffer.h"

ChunkIpv4::ChunkIpv4 ()
	: m_ver_ihl (4 | (20 << 4)),
	  m_tos (0),
	  m_id (0),
	  m_total_length (20),
	  m_fragment_offset (0),
	  m_ttl (0),
	  m_protocol (0),
	  m_source (0),
	  m_destination (0)
{}
ChunkIpv4::~ChunkIpv4 ()
{}

void 
ChunkIpv4::set_payload_size (uint16_t size)
{
	uint16_t old_total = utils_nstoh (m_total_length);
	uint16_t new_total = old_total + get_size ();
	m_total_length = utils_htons (new_total);
}
uint16_t 
ChunkIpv4::get_payload_size (void)
{
	uint16_t payload = utils_nstoh (m_total_length) - get_size ();
	return payload;
}


void 
ChunkIpv4::set_tos (uint8_t tos)
{
	m_tos = tos;
}
uint8_t 
ChunkIpv4::get_tos (void)
{
	return m_tos;
}
void 
ChunkIpv4::set_id (uint16_t id)
{
	m_id = utils_htons (id);
}
uint16_t 
ChunkIpv4::get_id (void)
{
	return utils_nstoh (m_id);
}
void 
ChunkIpv4::set_more_fragments (void)
{
	uint16_t fragment_offset = utils_nstoh (m_fragment_offset) >> 3;
	uint8_t flags = utils_nstoh (m_fragment_offset) & 0x7;
	flags |= 0x4;
	uint16_t new_fragment_offset = flags | (fragment_offset << 3);
	m_fragment_offset = utils_htons (new_fragment_offset);
}
void
ChunkIpv4::set_last_fragment (void)
{
	uint16_t fragment_offset = utils_nstoh (m_fragment_offset) >> 3;
	uint8_t flags = utils_nstoh (m_fragment_offset) & 0x7;
	flags &= ~0x4;
	uint16_t new_fragment_offset = flags | (fragment_offset << 3);
	m_fragment_offset = utils_htons (new_fragment_offset);
}
bool 
ChunkIpv4::is_last_fragment (void)
{
	uint8_t flags = utils_nstoh (m_fragment_offset) & 0x7;
	if (flags & 0x4) {
		return false;
	} else {
		return true;
	}
}
void 
ChunkIpv4::set_fragment_offset (uint16_t offset)
{
	uint16_t flags = utils_nstoh (m_fragment_offset) & 0x7;
	uint16_t new_fragment_offset = flags | (offset << 3);
	m_fragment_offset = utils_htons (new_fragment_offset);
}
uint16_t 
ChunkIpv4::get_fragment_offset (void)
{
	uint16_t fragment_offset = utils_nstoh (m_fragment_offset) >> 3;
	return fragment_offset;
}

void 
ChunkIpv4::set_ttl (uint8_t ttl)
{
	m_ttl = ttl;
}
uint8_t 
ChunkIpv4::get_ttl (void)
{
	return m_ttl;
}
	
uint8_t 
ChunkIpv4::get_protocol (void)
{
	return m_protocol;
}
void 
ChunkIpv4::set_protocol (uint8_t protocol)
{
	m_protocol = protocol;
}

void 
ChunkIpv4::set_source (Ipv4Address source)
{
	m_source = utils_htons (source.get_host_order ());
}
Ipv4Address
ChunkIpv4::get_source (void)
{
	return Ipv4Address (utils_nstoh (m_source));
}

void 
ChunkIpv4::set_destination (Ipv4Address dst)
{
	m_destination = utils_htons (dst.get_host_order ());
}
Ipv4Address
ChunkIpv4::get_destination (void)
{
	return Ipv4Address (utils_nstoh (m_destination));
}


uint32_t 
ChunkIpv4::get_size (void)
{
	uint8_t ihl = (m_ver_ihl >> 4) & 0x0f;
	uint32_t size = ihl * 4;
	return size;
}

Chunk *
ChunkIpv4::copy (void)
{
	ChunkIpv4 *ipv4 = new ChunkIpv4 (*this);
	return ipv4;
}

bool
ChunkIpv4::is_checksum_ok (void)
{
	uint16_t checksum = calculate_checksum ((uint8_t *)&m_ver_ihl, 20);
	if (checksum == 0xffff) {
		return true;
	} else {
		return false;
	}
}

void 
ChunkIpv4::serialize (WriteBuffer *buffer)
{
	// XXX 
	uint16_t checksum = utils_htons (calculate_checksum ((uint8_t *)&m_ver_ihl, 20));

	buffer->write_u8 (m_ver_ihl);
	buffer->write_u8 (m_tos);
	buffer->write ((uint8_t*)&m_total_length, 2);
	buffer->write ((uint8_t*)&m_id, 2);
	buffer->write ((uint8_t*)&m_fragment_offset, 2);
	buffer->write_u8 (m_ttl);
	buffer->write_u8 (m_protocol);
	buffer->write ((uint8_t*)&checksum, 2);
	buffer->write ((uint8_t*)&m_source, 4);
	buffer->write ((uint8_t*)&m_destination, 4);
}

void 
ChunkIpv4::deserialize (ReadBuffer *buffer)
{
	m_ver_ihl = buffer->read_u8 ();
	m_tos = buffer->read_u8 ();
	buffer->read ((uint8_t*)&m_total_length, 2);
	buffer->read ((uint8_t*)&m_id, 2);
	buffer->read ((uint8_t*)&m_fragment_offset, 2);
	m_ttl = buffer->read_u8 ();
	m_protocol = buffer->read_u8 ();
	buffer->read ((uint8_t*)&m_checksum, 2);
	buffer->read ((uint8_t*)&m_source, 4);
	buffer->read ((uint8_t*)&m_destination, 4);
}

void 
ChunkIpv4::print (std::ostream *os)
{
	// ipv4, right ?
	assert ((m_ver_ihl & 0x0f) == 4);
	*os << "ipv4 --";
	*os << " ihl: " << ((m_ver_ihl >> 4) & 0x0f);
	*os << " tos: " << m_tos;
	*os << " total length: " << m_total_length;
	*os << " id: " << m_id;
	*os << " flags: " << (m_fragment_offset & 0x7);
	*os << " frag offset: " << (m_fragment_offset>>3);
	*os << " ttl: " << m_ttl;
	*os << " protocol: " << m_protocol;
	*os << " checksum: " << m_checksum;
	*os << " source: " << m_source;
	*os << " destination: " << m_destination;
}
