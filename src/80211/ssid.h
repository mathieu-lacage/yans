/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef SSID_H
#define SSID_H

#include <stdint.h>

namespace yans {

class Ssid {
public:
	/* 0-terminated string */
	Ssid ();
	Ssid (char const *ssid);
	Ssid (char const ssid[32], uint8_t length);

	bool is_equal (Ssid const &o) const;
	bool is_broadcast (void) const;

	uint32_t get_length (void) const;
	void peek (uint8_t[32]) const;
	void set (uint8_t[32], uint32_t len);

	static Ssid get_broadcast (void);
private:
	char m_ssid[33];
	uint8_t m_length;
};

}; // namespace yans

#endif /* SSID_H */
