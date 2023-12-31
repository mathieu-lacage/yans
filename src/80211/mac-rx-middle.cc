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
#include "yans/packet.h"
#include "chunk-mac-80211-hdr.h"

#include <cassert>
#include <list>

#define noRX_MIDDLE_TRACE 1

#ifdef RX_MIDDLE_TRACE
#include <iostream>
# define TRACE(x) \
	std::cout <<"RX MIDDLE "<<x<<std::endl;
#else /* RX_MIDDLE_TRACE */
# define TRACE(x)
#endif /* RX_MIDDLE_TRACE */

namespace yans {


class OriginatorRxStatus {
private:
	typedef std::list<Packet> Fragments;
	typedef std::list<Packet>::const_iterator FragmentsCI;

	bool m_defragmenting;
	uint16_t m_last_sequence_control;
	Fragments m_fragments;
public:
	OriginatorRxStatus () {
		/* this is a magic value necessary. */
		m_last_sequence_control = 0xffff;
		m_defragmenting = false;
	}
	~OriginatorRxStatus () {
		for (FragmentsCI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		}
		m_fragments.erase (m_fragments.begin (), m_fragments.end ());
	}
	bool is_de_fragmenting (void) {
		return m_defragmenting;
	}
	void accumulate_first_fragment (Packet const packet) {
		assert (!m_defragmenting);
		m_defragmenting = true;
		m_fragments.push_back (packet);
	}
	Packet accumulate_last_fragment (Packet const packet) {
		assert (m_defragmenting);
		m_fragments.push_back (packet);
		m_defragmenting = false;
		Packet full;
		for (FragmentsCI i = m_fragments.begin (); i != m_fragments.end (); i++) {
			full.add_at_end (*i);
		}
		m_fragments.erase (m_fragments.begin (), m_fragments.end ());
		return full;
	}
	void accumulate_fragment (Packet const packet) {
		assert (m_defragmenting);
		m_fragments.push_back (packet);
	}
	bool is_next_fragment (uint16_t sequence_control) {
		if ((sequence_control >> 4) == (m_last_sequence_control >> 4) &&
		    (sequence_control & 0x0f) == ((m_last_sequence_control & 0x0f)+1)) {
			return true;
		} else {
			return false;
		}
	}
	uint16_t get_last_sequence_control (void) {
		return m_last_sequence_control;
	}
	void set_sequence_control (uint16_t sequence_control) {
		m_last_sequence_control = sequence_control;
	}

};


MacRxMiddle::MacRxMiddle ()
{}

MacRxMiddle::~MacRxMiddle ()
{
	for (OriginatorsI i = m_originator_status.begin ();
	     i != m_originator_status.end (); i++) {
		delete (*i).second;
	}
	m_originator_status.erase (m_originator_status.begin (),
				   m_originator_status.end ());
	for (QosOriginatorsI i = m_qos_originator_status.begin ();
	     i != m_qos_originator_status.end (); i++) {
		delete (*i).second;
	}
	m_qos_originator_status.erase (m_qos_originator_status.begin (),
				       m_qos_originator_status.end ());
}

void 
MacRxMiddle::set_forward_callback (ForwardUpCallback callback)
{
	m_callback = callback;
}

bool
MacRxMiddle::sequence_control_smaller (int seqca, int seqcb)
{
	int seqa = seqca >> 4;
	int seqb = seqcb >> 4;
	int delta = seqb - seqa;
	TRACE ("seqb="<<seqb<<", seqa="<<seqa<<", delta="<<delta);
	if (delta <= 0 && delta < -2048) {
		return true;
	} else if (delta >= 0 && delta < 2048) {
		return true;
	} else {
		return false;
	}
}


OriginatorRxStatus *
MacRxMiddle::lookup (ChunkMac80211Hdr const *hdr)
{
	OriginatorRxStatus *originator;
	MacAddress source = hdr->get_addr2 ();
	if (hdr->is_qos_data () &&
	    !hdr->get_addr2 ().is_broadcast ()) {
                /* only for qos data non-broadcast frames */
		originator = m_qos_originator_status[std::make_pair(source, hdr->get_qos_tid ())];
		if (originator == 0) {
			originator = new OriginatorRxStatus ();
			m_qos_originator_status[std::make_pair(source, hdr->get_qos_tid ())] = originator;
		}
	} else {
                /* - management frames
                 * - qos data broadcast frames
                 * - nqos data frames
                 * see section 7.1.3.4.1
                 */
		originator = m_originator_status[source];
		if (originator == 0) {
			originator = new OriginatorRxStatus ();
			m_originator_status[source] = originator;
		}
	}
	return originator;
}

bool
MacRxMiddle::is_duplicate (ChunkMac80211Hdr const*hdr, 
			   OriginatorRxStatus *originator) const
{
	if (originator->get_last_sequence_control () == hdr->get_sequence_control ()) {
		return true;
	}
	return false;
}

Packet 
MacRxMiddle::handle_fragments (Packet packet, ChunkMac80211Hdr const*hdr,
			       OriginatorRxStatus *originator, bool *complete)
{
	if (originator->is_de_fragmenting ()) {
		if (hdr->is_more_fragments ()) {
			if (originator->is_next_fragment (hdr->get_sequence_control ())) {
				TRACE ("accumulate fragment seq="<<hdr->get_sequence_number ()<<
				       ", frag="<<hdr->get_fragment_number ()<<
				       ", size="<<packet.get_size ());
				originator->accumulate_fragment (packet);
				originator->set_sequence_control (hdr->get_sequence_control ());
			} else {
				TRACE ("non-ordered fragment");
			}
			*complete = false;
			return Packet ();
		} else {
			if (originator->is_next_fragment (hdr->get_sequence_control ())) {
				TRACE ("accumulate last fragment seq="<<hdr->get_sequence_number ()<<
				       ", frag="<<hdr->get_fragment_number ()<<
				       ", size="<<hdr->get_size ());
				packet = originator->accumulate_last_fragment (packet);
				originator->set_sequence_control (hdr->get_sequence_control ());
				*complete = true;
				return packet;
			} else {
				TRACE ("non-ordered fragment");
				*complete = false;
				return Packet ();
			}
		}
	} else {
		if (hdr->is_more_fragments ()) {
			TRACE ("accumulate first fragment seq="<<hdr->get_sequence_number ()<<
			       ", frag="<<hdr->get_fragment_number ()<<
			       ", size="<<packet.get_size ());
			originator->accumulate_first_fragment (packet);
			originator->set_sequence_control (hdr->get_sequence_control ());
			*complete = false;
			return Packet ();
		} else {
			*complete = true;
			return packet;
		}
	}
}

void
MacRxMiddle::receive (Packet packet, ChunkMac80211Hdr const *hdr)
{
	OriginatorRxStatus *originator = lookup (hdr);
	if (hdr->is_data ()) {
		assert (sequence_control_smaller (originator->get_last_sequence_control (), 
						  hdr->get_sequence_control ()));
		// filter duplicates.
		if (is_duplicate (hdr, originator)) {
			TRACE ("duplicate from="<<hdr->get_addr2 ()<<
			       ", seq="<<hdr->get_sequence_number ()<<
			       ", frag="<<hdr->get_fragment_number ());
			return;
		}
		bool complete;
		Packet agregate = handle_fragments (packet, hdr, originator, &complete);
		if (!complete) {
			return;
		}
		TRACE ("forwarding data from="<<hdr->get_addr2 ()<<
		       ", seq="<<hdr->get_sequence_number ()<<
		       ", frag="<<hdr->get_fragment_number ());
		originator->set_sequence_control (hdr->get_sequence_control ());
		m_callback (agregate, hdr);
	} else {
		TRACE ("forwarding "<<hdr->get_type_string ()<<
		       ", seq="<<hdr->get_sequence_number ()<<
		       ", frag="<<hdr->get_fragment_number ());
		originator->set_sequence_control (hdr->get_sequence_control ());
		m_callback (packet, hdr);
	}
}

}; // namespace yans
