/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef IPV4_ADDRESS_H
#define IPV4_ADDRESS_H

#include <stdint.h>

/* Ipv4 addresses are stored in host order in
 * this class.
 */
class Ipv4Address {
public:
	Ipv4Address ();
	/* input address is in host order. */
	Ipv4Address (uint32_t address);
	/* input address is in format:
	 * hhh.xxx.xxx.lll
	 * where h is the high byte and l the
	 * low byte
	 */
	Ipv4Address (char const *address);

	bool is_equal (Ipv4Address other) const;

	/* Using this method is frowned upon.
	 * Please, do _not_ use this method.
	 * It is there only for chunk-ipv4.
	 */
	uint32_t get_host_order (void);

	static Ipv4Address get_zero (void);
	static Ipv4Address get_broadcast (void);
	static Ipv4Address get_loopback (void);
private:
	uint32_t m_address;
	static Ipv4Address m_zero;
	static Ipv4Address m_broadcast;
	static Ipv4Address m_loopback;
};


class Ipv4Mask {
public:
	Ipv4Mask ();
	Ipv4Mask (uint32_t mask);
	Ipv4Mask (char const *mask);

	bool is_match (Ipv4Address a, Ipv4Address b);

	bool is_equal (Ipv4Mask other) const;

	static Ipv4Mask get_loopback (void);
	static Ipv4Mask get_zero (void);
private:
	static Ipv4Mask m_loopback;
	static Ipv4Mask m_zero;
	uint32_t m_mask;
};

#endif /* IPV4_ADDRESS_H */
