/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <stdint.h>

class WriteBuffer;
class ReadBuffer;

class MacAddress {
public:
	/* low byte should be first.
	 */
	MacAddress (uint8_t address[6]);
	/* The string should look like this:
	 * xx-xx-xx-xx-xx-xx
	 */
	MacAddress (char const *address);
	/* create the mac address associated to 
	 * this multicast ip address. 
	 */
	MacAddress (uint32_t multicast_ip_address);
	~MacAddress ();

	bool is_equal (MacAddress *other);
	bool is_broadcast (void);
	bool is_multicast (void);
	bool is_multicast_equal (MacAddress *other);

	void serialize (WriteBuffer *buffer);
	void deserialize (ReadBuffer *buffer);

private:
	uint32_t get_multicast_part (void);
	uint8_t m_address[6];
};


#endif /* MAC_ADDRESS_H */
