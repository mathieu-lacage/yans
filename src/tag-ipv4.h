/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef TAG_IPV4_H
#define TAG_IPV4_H

#include "tag-manager.h"
#include "ipv4-address.h"
#include "ipv4-route.h"

class NetworkInterface;

class TagOutIpv4 : public Tag {
public:
	TagOutIpv4 (Route const *route, uint16_t sport, uint16_t dport);

	uint16_t get_dport (void);
	uint16_t get_sport (void);

	Route const *get_route (void);

	static uint32_t get_tag (void);
 private:
	Route m_route;
	uint16_t m_sport;
	uint16_t m_dport;
	static uint32_t m_tag;
};

#endif /* TAG_IPV4_H */
