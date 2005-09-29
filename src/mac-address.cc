/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "mac-address.h"
#include "buffer.h"

MacAddress::MacAddress (uint8_t address[6])
{
	m_address[0] = address[0];
	m_address[1] = address[1];
	m_address[2] = address[2];
	m_address[3] = address[3];
	m_address[4] = address[4];
	m_address[5] = address[5];
}
MacAddress::MacAddress (char const *address)
{
	assert (false);
	// XXX: implement.
}
MacAddress::MacAddress (uint32_t ip_multicast_address)
{
	m_address[0] = 1 | ((ip_multicast_address & 0x7f) << 1);
	m_address[1] = (ip_multicast_address >> 7) & 0xff;
	m_address[2] = (ip_multicast_address >> 16) & 0xff;
	m_address[3] = 0x5e;
	m_address[4] = 0;
	m_address[5] = 0;
}
MacAddress::~MacAddress ()
{}

bool 
MacAddress::is_equal (MacAddress *other)
{
	if (m_address[0] == other->m_address[0] &&
	    m_address[1] == other->m_address[1] &&
	    m_address[2] == other->m_address[2] &&
	    m_address[3] == other->m_address[3] &&
	    m_address[4] == other->m_address[4] &&
	    m_address[5] == other->m_address[5]) {
		return true;
	} else {
		return false;
	}
}
bool 
MacAddress::is_broadcast (void)
{
	if (m_address[0] == 0xff &&
	    m_address[1] == 0xff &&
	    m_address[2] == 0xff &&
	    m_address[3] == 0xff &&
	    m_address[4] == 0xff &&
	    m_address[5] == 0xff) {
		return true;
	} else {
		return false;
	}
}
bool 
MacAddress::is_multicast (void)
{
	if (m_address[0] & 0x1) {
		return true;
	} else {
		return false;
	}
}
bool 
MacAddress::is_multicast_equal (MacAddress *other)
{
	if (get_multicast_part () == other->get_multicast_part ()) {
		return true;
	} else {
		return false;
	}
}

uint32_t
MacAddress::get_multicast_part (void)
{
	uint32_t part = 0;
	part |= m_address[0] >> 1;
	part |= m_address[1] << 7;
	part |= (m_address[1] << 15);
	return part;
}

void 
MacAddress::serialize (WriteBuffer *buffer)
{
	buffer->write (m_address, 6);
}
void 
MacAddress::deserialize (ReadBuffer *buffer)
{
	buffer->read (m_address, 6);
}
