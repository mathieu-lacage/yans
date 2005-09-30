/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TAG_IPV4_H
#define TAG_IPV4_H

#include "tag-manager.h"

class TagIpv4 : public Tag {
public:
	Ipv4Address m_sip;
	Ipv4Address m_dip;
	uint16_t m_sport;
	uint16_t m_dport;

	static uint32_t get_tag (void);
 private:
	static uint32_t m_tag;
};

#endif /* TAG_IPV4_H */
