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

#include "supported-rates.h"
#include "buffer.h"

#include <cassert>

namespace yans {

SupportedRates::SupportedRates ()
	: m_n_rates (0)
{}

void 
SupportedRates::add_supported_rate (uint32_t bs)
{
	assert (m_n_rates <= 8);
	m_rates[m_n_rates] = bs/500000;
	m_n_rates++;
}
bool 
SupportedRates::is_supported_rate (uint32_t bs) const
{
	uint8_t rate = bs / 500000;
	for (uint8_t i = 0; i < m_n_rates; i++) {
		if (rate == m_rates[i]) {
			return true;
		}
	}
	return false;
}

uint32_t 
SupportedRates::get_size (void) const
{
	return 1 + 1 + m_n_rates;
}
void 
SupportedRates::write_to (Buffer *buffer) const
{
	buffer->write_u8 (1); // supported rates element id
	buffer->write_u8 (m_n_rates);
	for (uint8_t i = 0; i < m_n_rates; i++) {
		buffer->write_u8 (m_rates[i]);
	}
}
uint32_t 
SupportedRates::read_from (Buffer *buffer)
{
	//uint8_t rates_id = buffer->read_u8 ();
	//assert (rates_id == 1);
	buffer->read_u8 ();
	m_n_rates = buffer->read_u8 ();
	assert (m_n_rates <= 8);
	for (uint8_t i = 0; i < m_n_rates; i++) {
		m_rates[i] = buffer->read_u8 ();
	}
	return get_size ();
}

}; // namespace yans
