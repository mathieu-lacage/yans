/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef CHUNK_ARP_H
#define CHUNK_ARP_H

#include "chunk.h"
#include "mac-address.h"
#include "ipv4-address.h"

class ChunkArp : public Chunk {
 public:
	virtual ~ChunkArp ();

	void set_request (MacAddress source_hardware_address,
			  Ipv4Address source_protocol_address,
			  Ipv4Address destination_protocol_address);
	void set_reply (MacAddress source_hardware_address,
			Ipv4Address source_protocol_address,
			MacAddress destination_hardware_address,
			Ipv4Address destination_protocol_address);
	bool is_request (void);
	bool is_reply (void);
	MacAddress get_source_hardware_address (void);
	MacAddress get_destination_hardware_address (void);
	Ipv4Address get_source_ipv4_address (void);
	Ipv4Address get_destination_ipv4_address (void);

	virtual uint32_t get_size (void);
	virtual Chunk *copy (void);
	virtual void serialize (WriteBuffer *buffer);
	virtual void deserialize (ReadBuffer *buffer);
	virtual void print (std::ostream *os);

private:
	enum ArpType_e {
		ARP_TYPE_REQUEST = 1,
		ARP_TYPE_REPLY   = 2
	};
	uint16_t m_type;
	MacAddress m_mac_source;
	MacAddress m_mac_dest;
	Ipv4Address m_ipv4_source;
	Ipv4Address m_ipv4_dest;
};

#endif /* CHUNK_ARP_H */
