/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef MAC_ADDRESS_H
#define MAC_ADDRESS_H

#include <stdint.h>
#include <ostream>

class WriteBuffer;
class ReadBuffer;

class MacAddress {
public:
	MacAddress (void);
	/* low byte should be first.
	 */
	MacAddress (uint8_t address[6]);
	/* The string should look like this:
	 * hh-xx-xx-xx-xx-ll
	 * where hh is the high byte and ll is
	 * the low byte.
	 */
	MacAddress (char const *address);
	/* create the mac address associated to 
	 * this multicast ip address. 
	 */
	MacAddress (uint32_t multicast_ip_address);
	~MacAddress ();

	bool is_equal (MacAddress other) const;
	bool is_broadcast (void) const;
	bool is_multicast (void) const;
	bool is_multicast_equal (MacAddress other) const;

	void serialize (WriteBuffer *buffer) const;
	void deserialize (ReadBuffer *buffer);
	void print (std::ostream *os) const;

	static MacAddress get_broadcast (void);
private:
	static MacAddress m_broadcast;
	uint32_t get_multicast_part (void) const;
	uint8_t m_address[6];
};

#endif /* MAC_ADDRESS_H */
