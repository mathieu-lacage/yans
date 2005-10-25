/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "chunk-arp.h"
#include "buffer.h"

ChunkArp::~ChunkArp ()
{}

void 
ChunkArp::set_request (MacAddress source_hardware_address,
		       Ipv4Address source_protocol_address,
		       Ipv4Address destination_protocol_address)
{
	m_type = ARP_TYPE_REQUEST;
	m_mac_source = source_hardware_address;
	m_mac_dest = MacAddress::get_broadcast ();
	m_ipv4_source = source_protocol_address;
	m_ipv4_dest = destination_protocol_address;
}
void 
ChunkArp::set_reply (MacAddress source_hardware_address,
		     Ipv4Address source_protocol_address,
		     MacAddress destination_hardware_address,
		     Ipv4Address destination_protocol_address)
{
	m_type = ARP_TYPE_REPLY;
	m_mac_source = source_hardware_address;
	m_mac_dest = destination_hardware_address;
	m_ipv4_source = source_protocol_address;
	m_ipv4_dest = destination_protocol_address;
}
bool 
ChunkArp::is_request (void)
{
	return (m_type == ARP_TYPE_REQUEST)?true:false;
}
bool 
ChunkArp::is_reply (void)
{
	return (m_type == ARP_TYPE_REPLY)?true:false;
}
MacAddress 
ChunkArp::get_source_hardware_address (void)
{
	return m_mac_source;
}
MacAddress 
ChunkArp::get_destination_hardware_address (void)
{
	return m_mac_dest;
}
Ipv4Address 
ChunkArp::get_source_ipv4_address (void)
{
	return m_ipv4_source;
}
Ipv4Address 
ChunkArp::get_destination_ipv4_address (void)
{
	return m_ipv4_dest;
}


uint32_t 
ChunkArp::get_size (void)
{
	/* this is the size of an ARP payload. */
	return 28;
}
Chunk *
ChunkArp::copy (void)
{
	ChunkArp *chunk = new ChunkArp ();
	*chunk = *this;
	return chunk;
}

void 
ChunkArp::serialize (WriteBuffer *buffer)
{
	// XXX should be careful with network/host order.
	/* ethernet */
	buffer->write_hton_u16 (0x0001);
	/* ipv4 */
	buffer->write_hton_u16 (0x0800);
	buffer->write_u8 (6);
	buffer->write_u8 (4);
	buffer->write_hton_u16 (m_type);
	m_mac_source.serialize (buffer);
	m_ipv4_source.serialize (buffer);
	m_mac_dest.serialize (buffer);
	m_ipv4_dest.serialize (buffer);
}
void 
ChunkArp::deserialize (ReadBuffer *buffer)
{
	uint16_t htype = buffer->read_ntoh_u16 ();
	assert (htype == 0x0001);
	uint16_t ptype = buffer->read_ntoh_u16 ();
	assert (ptype == 0x0800);
	uint8_t hlen = buffer->read_u8 ();
	assert (hlen == 6);
	uint8_t plen = buffer->read_u8 ();
	assert (plen == 4);
	m_type = buffer->read_ntoh_u16 ();
	m_mac_source.deserialize (buffer);
	m_ipv4_source.deserialize (buffer);
	m_mac_dest.deserialize (buffer);
	m_ipv4_dest.deserialize (buffer);
}
void 
ChunkArp::print (std::ostream *os)
{
	*os << "(arp)";
	if (is_request ()) {
		*os << " source mac: ";
		m_mac_source.print (os);
		*os << " source ipv4: ";
		m_ipv4_source.print (os);
		*os << " dest ipv4: ";
		m_ipv4_dest.print (os);
	} else {
		assert (is_reply ());
		*os << " source mac: ";
		m_mac_source.print (os);
		*os << " source ipv4: ";
		m_ipv4_source.print (os);
		*os << " dest mac: ";
		m_mac_dest.print (os);
		*os << " dest ipv4: ";
		m_ipv4_dest.print (os);
	}
}

