/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include <cassert>

#include "mac-tx-middle.h"
#include "chunk-mac-80211-hdr.h"

namespace yans {

MacTxMiddle::MacTxMiddle ()
{
	m_sequence = 0;
	for (uint8_t i = 0; i < 16; i++) {
		m_qos_sequences[i] = 0;
	}
}

uint16_t 
MacTxMiddle::get_next_sequence_number_for (ChunkMac80211Hdr const*hdr)
{
	uint16_t retval;
	if (hdr->is_qos_data () &&
	    !hdr->get_addr1 ().is_broadcast ()) {
		uint8_t tid = hdr->get_qos_tid ();
		assert (tid < 16);
		retval = m_qos_sequences[tid];
		m_qos_sequences[tid]++;
		m_qos_sequences[tid] %= 4096;
	} else {
		retval = m_sequence;
		m_sequence++;
		m_sequence %= 4096;
	}
	return retval;
}

}; // namespace yans
