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

#include "mac-rx-middle.h"
#include "mac-low.h"
#include "mac-high.h"
#include "mac-station.h"
#include "mac-traces.h"
#include "net-interface-80211.h"
#include "common.h"

#include "packet.h"

#define nopeRX_MIDDLE_TRACE 1

#ifdef RX_MIDDLE_TRACE
#include <iostream>
# define TRACE(x) \
	printf ("RX MIDDLE "<<x<<std::endl);
#else /* RX_MIDDLE_TRACE */
# define TRACE(x)
#endif /* RX_MIDDLE_TRACE */


class OriginatorRxStatus {
public:
	OriginatorRxStatus () {
		/* this is a magic value necessary. */
		m_last_sequence_control = 0xffff;
		m_de_fragmenting = false;
	}
	bool is_de_fragmenting (void) {
		return m_deFragmenting;
	}
	void accumulate_first_fragment (int firstFragmentSize) {
		assert (!m_deFragmenting);
		m_de_fragmenting = true;
		m_rx_size = firstFragmentSize;
	}
	int accumulate_last_fragment (int fragmentSize) {
		m_de_fragmenting = false;
		m_rx_size += fragmentSize;
		return m_rxSize;
	}
	void accumulate_fragment (int fragmentSize) {
		m_rx_size += fragmentSize;
	}
	bool is_next_fragment (uint16_t sequenceControl) {
		if ((sequenceControl >> 4) == (m_lastSequenceControl >> 4) &&
		    (sequenceControl & 0x0f) == ((m_lastSequenceControl & 0x0f)+1)) {
			return true;
		} else {
			return false;
		}
	}
	uint16_t get_last_sequence_control (void) {
		return m_lastSequenceControl;
	}
	void set_sequence_control (uint16_t sequenceControl) {
		m_last_sequence_control = sequenceControl;
	}

private:
	bool m_deFragmenting;
	uint16_t m_lastSequenceControl;
	int m_rxSize;
};


MacRxMiddle::MacRxMiddle ()
{}

void 
MacRxMiddle::set_interface (NetInterface80211 *interface)
{
	m_interface = interface;
}

void
MacRxMiddle::drop_packet (Packet *packet)
{
	Packet::free (packet);
}

bool
MacRxMiddle::sequence_control_smaller (int seqca, int seqcb)
{
	int seqa = seqca >> 4;
	int seqb = seqcb >> 4;
	int delta = seqb - seqa;
	TRACE ("seqb: %d, seqa: %d, delta: %d", seqb, seqa, delta);
	if (delta <= 0 && delta < -2048) {
		return true;
	} else if (delta >= 0 && delta < 2048) {
		return true;
	} else {
		return false;
	}
}


OriginatorRxStatus *
MacRxMiddle::lookup_qos (int source, int TID) 
{
	OriginatorRxStatus *originator;
	originator = m_qosOriginatorStatus[make_pair(source, TID)];
	if (originator == 0) {
		originator = new OriginatorRxStatus ();
		m_qosOriginatorStatus[make_pair(source, TID)] = originator;
	}
	return originator;
}

OriginatorRxStatus *
MacRxMiddle::lookup_nqos (int source) 
{
	OriginatorRxStatus *originator;
	originator = m_originatorStatus[source];
	if (originator == 0) {
		originator = new OriginatorRxStatus ();
		m_originatorStatus[source] = originator;
	}
	return originator;
}

OriginatorRxStatus *
MacRxMiddle::lookup (Packet *packet)
{
	OriginatorRxStatus *originator;
	if (isQos (packet) &&
            is_data (packet) &&
            get_destination (packet) != MAC_BROADCAST) {
                /* only for qos data broadcast frames */
		originator = lookupQos (getSource (packet), get_tid (packet));
	} else {
                /* - management frames
                 * - qos data broadcast frames
                 * - nqos data frames
                 * see section 7.1.3.4.1
                 */
		originator = lookup_nqos (getSource (packet));
	}
	return originator;
}

bool
MacRxMiddle::handle_duplicates (Packet *packet, OriginatorRxStatus *originator)
{
	if (originator->getLastSequenceControl () == get_sequence_control (packet)) {
		TRACE ("dump duplicate seq=0x%x tid=%u", 
		       get_sequence_control (packet),
		       get_tid (packet));
		dropPacket (packet);
		return true;
	}
	return false;
}

bool
MacRxMiddle::handle_fragments (Packet *packet, OriginatorRxStatus *originator)
{
	// defragment
	if (originator->is_de_fragmenting ()) {
		if (getMoreFragments (packet)) {
			if (originator->is_next_fragment (getSequenceControl (packet))) {
				TRACE ("accumulate fragment 0x%x %d", 
				       getSequenceControl (packet), get_size (packet));
				originator->accumulate_fragment (getSize (packet));
				originator->set_sequence_control (getSequenceControl (packet));
				dropPacket (packet);
			} else {
				TRACE ("drop invalid fragment");
				dropPacket (packet);
			}
		} else {
			if (originator->is_next_fragment (getSequenceControl (packet))) {
				/* Each fragment is a copy of the original non-fragmented
				 * packet except for its size which is set to the size of
				 * the real fragment. This allows us to re-use the last 
				 * fragment to change its size back to the original packet 
				 * size and to pass it up to the higher-level layers.
				 */
				TRACE ("accumulate last fragment 0x%x %d", 
				       getSequenceControl (packet), get_size (packet));
				int finalSize = originator->accumulate_last_fragment (getSize (packet));
				setSize (packet, finalSize);
				originator->set_sequence_control (getSequenceControl (packet));
				// pass-through to give to mac high.
				return false;
			} else {
				TRACE ("drop packet");
				dropPacket (packet);
			}
		}
	} else {
		if (getMoreFragments (packet)) {
			TRACE ("accumulate first fragment 0x%x %d", 
			       getSequenceControl (packet), get_size (packet));
			originator->accumulate_first_fragment (getSize (packet));
			originator->set_sequence_control (getSequenceControl (packet));
			dropPacket (packet);
		} else {
			TRACE ("no defrag -- passthrough");
			// pass-through to give to mac high.
			return false;
		}
	}
	return true;
}

void
MacRxMiddle::send_up (Packet *packet)
{
	OriginatorRxStatus *originator = lookup (packet);
	switch (getType (packet)) {
	case MAC_80211_MGT_ADDBA_REQUEST:
		TRACE ("got addba req");
		originator->set_sequence_control (getSequenceControl (packet));
		dropPacket (packet);
		break;
	case MAC_80211_DATA:
		assert (sequenceControlSmaller (originator->getLastSequenceControl (), get_sequence_control (packet)));
		// filter duplicates.
		if (!handleDuplicates (packet, originator) &&
		    //!handleBlockAck (packet, originator) &&
		    !handleFragments (packet, originator)) {
			TRACE ("forwarding data from %d seq=0x%x tid=%u", 
			       getSource (packet), get_sequence_control (packet),
			       get_tid (packet));
			originator->set_sequence_control (getSequenceControl (packet));
			m_interface->high ()->receive_from_mac_low (packet);
		} else {
			dropPacket (packet);
		}
		break;
	default:
		TRACE ("forwarding %s seq=0x%x tid=%u",
		       get_type_string (packet), 
		       get_sequence_control (packet),
		       get_tid (packet));
		originator->set_sequence_control (getSequenceControl (packet));
		m_interface->high ()->receive_from_mac_low (packet);
		break;
	}
}
