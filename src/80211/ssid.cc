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
#include "ssid.h"
#include <cassert>

namespace yans {

Ssid::Ssid ()
{
	for (uint8_t i = 0; i < 33; i++) {
		m_ssid[i] = 0;
	}
}
Ssid::Ssid (char const *ssid)
{
	uint8_t len = 0;
	while (*ssid != 0 && len < 32) {
		m_ssid[len] = *ssid;
		ssid++;
		len++;
	}
	while (len < 33) {
		m_ssid[len] = 0;
		len++;
	}
}
Ssid::Ssid (char const ssid[32], uint8_t length)
{
	assert (length <= 32);
	uint8_t len = 0;
	while (len < length) {
		m_ssid[len] = ssid[len];
		len++;
	}
	while (len < 33) {
		m_ssid[len] = 0;
		len++;
	}
}
bool 
Ssid::is_equal (Ssid const &o) const
{
	uint8_t i = 0;
	while (i < 32 && 
	       m_ssid[i] == o.m_ssid[i] &&
	       m_ssid[i] != 0) {
		i++;
	}
	if (m_ssid[i] != o.m_ssid[i]) {
		return false;
	}
	return true;
}
bool 
Ssid::is_broadcast (void) const
{
	if (m_ssid[0] == 0) {
		return true;
	}
	return false;
}
uint32_t 
Ssid::get_length (void) const
{
	uint8_t size = 0;
	while (m_ssid[size] != 0 && size < 32) {
		size++;
	}
	assert (size <= 32);
	return size;
}

void
Ssid::peek (uint8_t ssid[32]) const
{
	for (uint8_t i = 0; i < 32; i++) {
		ssid[i] = m_ssid[i];
	}
}
void 
Ssid::set (uint8_t ssid[32], uint32_t len)
{
	assert (len < 32);
	for (uint8_t i = 0; i < len; i++) {
		m_ssid[i] = ssid[i];
	}	
}

Ssid 
Ssid::get_broadcast (void)
{
	return Ssid ("");
}

}; // namespace yans
