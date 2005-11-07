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
#include "chunk-piece.h"


DefragState::DefragState ()
{
	m_defrag_start = Simulator::instance ()->now_s ();
}
DefragState::~DefragState ()
{
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		(*i)->unref ();
	}
	m_fragments.erase (m_fragments.begin (), m_fragments.end ());
}

void 
DefragState::add (Packet *fragment)
{
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		ChunkIpv4 *ip = static_cast <ChunkIpv4 *> ((*i)->peek_header ());
		ChunkIpv4 *ip_frag = static_cast <ChunkIpv4 *> (fragment->peek_header ());
		assert (ip->get_identification () == ip_frag->get_identification ());
		assert (ip->get_source () == ip_frag->get_source ());
		assert (ip->get_destination () == ip_frag->get_destination ());
		assert (ip->get_protocol () == ip_frag->get_protocol ());
		if (ip->get_fragment_offset () > ip_frag->get_fragment_offset ()) {
			fragment->ref ();
			m_fragments.insert (i, fragment);
			return;
		}
	}
	fragment->ref ();
	m_fragments.push_back (fragment);
}
bool 
DefragState::is_complete (void)
{
	uint32_t expected_offset = 0;
	for (FragmentsI i = m_fragments.begin (); i != m_fragments.end (); i++) {
		ChunkIpv4 *ip = static_cast <ChunkIpv4 *> ((*i)->peek_header ());
		if (ip->get_fragment_offset () > expected_offset) {
			/* there is a hole. */
			return false;
		}
		expected_offset += ip->get_payload_size ();
	}
	Packet *last_fragment = m_fragments.back ();
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (last_fragment->peek_header ());
	if (ip->is_last_fragment ()) {
		return true;
	}
	return false;
}
Packet *
DefragState::get_complete (void)
{
	Packet *first_fragment = m_fragments.front ();
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (first_fragment->remove_header ());
	ChunkPiece *first_piece = static_cast <ChunkPiece *> (first_fragment->peek_header ());
	first_fragment->add_header (ip);

	Packet *original = first_piece->get_original ();
	return original;
}

bool 
DefragState::is_too_old (void)
{
	double now = Simulator::instance ()->now_s ();
	if (now - m_defrag_start > get_reassembly_timeout ()) {
		return true;
	} else {
		return false;
	}
}
bool 
DefragState::matches (Packet *fragment)
{
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (fragment->peek_header ());
	if (ip->get_source () == get_source () &&
	    ip->get_destination () == get_destination () &&
	    ip->get_protocol () == get_protocol () &&
	    ip->get_identification () == get_identification ()) {
		return true;
	} else {
		return false;
	}
}
Ipv4Address 
DefragState::get_source (void)
{
	assert (!m_fragments.empty ());
	Packet *first_fragment = *m_fragments.begin ();
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (first_fragment->peek_header ());
	return ip->get_source ();
}
Ipv4Address 
DefragState::get_destination (void)
{
	assert (!m_fragments.empty ());
	Packet *first_fragment = *m_fragments.begin ();
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (first_fragment->peek_header ());
	return ip->get_destination ();
}
uint8_t 
DefragState::get_protocol (void)
{
	assert (!m_fragments.empty ());
	Packet *first_fragment = *m_fragments.begin ();
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (first_fragment->peek_header ());
	return ip->get_protocol ();
}
uint16_t 
DefragState::get_identification (void)
{
	assert (!m_fragments.empty ());
	Packet *first_fragment = *m_fragments.begin ();
	ChunkIpv4 *ip = static_cast <ChunkIpv4 *> (first_fragment->peek_header ());
	return ip->get_identification ();
}

double
DefragState::get_reassembly_timeout (void)
{
	return 60.0;
}




DefragStates::DefragStates ()
{}
DefragStates::~DefragStates ()
{}

DefragState *
DefragStates::lookup (Packet *fragment)
{
	StatesI i = m_states.begin ();
	while (i != m_states.end ()) {
		DefragState *state = *i;
		if (state->is_too_old ()) {
			delete state;
			i = m_states.erase (i);
			continue;
		}
		if (state->matches (fragment)) {
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
