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


#include "defrag-state.h"
#include "simulator.h"
#include "packet.h"
#include "chunk-ipv4.h"

namespace yans {


DefragFragment::DefragFragment (Packet *fragment, ChunkIpv4 *ip)
	: m_fragment (fragment),
	  m_is_last (ip->is_last_fragment ()),
	  m_offset (ip->get_fragment_offset ())
{}
Packet *
DefragFragment::get_fragment (void)
{
	return m_fragment;
}
bool 
DefragFragment::is_last (void)
{
	return m_is_last;
}
uint16_t 
DefragFragment::get_offset (void)
{
	return m_offset;
}
uint32_t 
DefragFragment::get_size (void)
{
	return m_fragment->get_size ();
}


DefragState::DefragState (ChunkIpv4 const *ip)
	: m_source (ip->get_source ()),
	  m_destination (ip->get_destination ()),
	  m_protocol (ip->get_protocol ()),
	  m_identification (ip->get_identification ())
{
	m_defrag_start = Simulator::now_s ();
}
DefragState::~DefragState ()
{
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		(*i).get_fragment ()->unref ();
	}
	m_fragments.erase (m_fragments.begin (), m_fragments.end ());
}

void 
DefragState::add (Packet *fragment, ChunkIpv4 *ip_frag)
{
	assert (m_identification == ip_frag->get_identification ());
	assert (m_source == ip_frag->get_source ());
	assert (m_destination == ip_frag->get_destination ());
	assert (m_protocol == ip_frag->get_protocol ());
	
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		if ((*i).get_offset () > ip_frag->get_fragment_offset ()) {
			fragment->ref ();
			m_fragments.insert (i, DefragFragment (fragment, ip_frag));
			return;
		}
	}
	fragment->ref ();
	m_fragments.push_back (DefragFragment (fragment, ip_frag));
}
bool 
DefragState::is_complete (void)
{
	uint32_t expected_offset = 0;
	bool complete = false;
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		if (complete) {
			/* there is a fragment after the last fragment. 
			 *  This sounds fishy.
			 */
			return false;
		}
		if ((*i).get_offset () > expected_offset) {
			/* there is a hole. */
			return false;
		}
		if ((*i).is_last ()) {
			complete = true;
		}
		expected_offset += (*i).get_size ();
	}
	return complete;
}
Packet *
DefragState::get_complete (void)
{
	assert (is_complete ());

	Packet *packet = new Packet ();
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		packet->add_at_end ((*i).get_fragment ());
	}
	return packet;
}

bool 
DefragState::is_too_old (void)
{
	double now = Simulator::now_s ();
	if (now - m_defrag_start > get_reassembly_timeout ()) {
		return true;
	} else {
		return false;
	}
}
bool 
DefragState::matches (ChunkIpv4 const *ip)
{
	if (ip->get_source () == m_source &&
	    ip->get_destination () == m_destination &&
	    ip->get_protocol () == m_protocol &&
	    ip->get_identification () == m_identification) {
		return true;
	} else {
		return false;
	}
}

double
DefragState::get_reassembly_timeout (void)
{
	return 60.0;
}




DefragStates::DefragStates ()
{}
DefragStates::~DefragStates ()
{
	for (StatesI i = m_states.begin (); i != m_states.end (); i++) {
		delete (*i);
	}
	m_states.erase (m_states.begin (), m_states.end ());
}

DefragState *
DefragStates::lookup (ChunkIpv4 const *ip)
{
	StatesI i = m_states.begin ();
	while (i != m_states.end ()) {
		DefragState *state = *i;
		if (state->is_too_old ()) {
			delete state;
			i = m_states.erase (i);
			continue;
		}
		if (state->matches (ip)) {
			return state;
		}
		i++;
	}
	return 0;
}
void 
DefragStates::add (DefragState *state)
{
	m_states.push_back (state);
}
void 
DefragStates::remove (DefragState *state)
{
	m_states.remove (state);
}

}; // namespace yans
