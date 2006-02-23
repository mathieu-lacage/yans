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

#ifndef DEFRAG_STATE_H
#define DEFRAG_STATE_H

#include <list>
#include "ipv4-address.h"

namespace yans {

class Packet;
class ChunkIpv4;

class DefragFragment {
public:
	DefragFragment (Packet *fragment, ChunkIpv4 *ip);
	Packet *get_fragment (void);
	bool is_last (void);
	uint16_t get_offset (void);
	uint32_t get_size (void);
private:
	Packet *m_fragment;
	bool m_is_last;
	uint16_t m_offset;
};

class DefragState {
public:
	DefragState (ChunkIpv4 const *ip);
	~DefragState ();

	void add (Packet *fragment, ChunkIpv4 *ip);
	bool is_complete (void);
	Packet *get_complete (void);

	bool is_too_old (void);
	bool matches (ChunkIpv4 const *ip);
private:
	uint64_t get_reassembly_timeout_us (void);

	typedef std::list <DefragFragment> Fragments;
	typedef std::list <DefragFragment>::iterator FragmentsI;

	uint64_t m_defrag_start_us;
	Fragments m_fragments;
	Ipv4Address m_source;
	Ipv4Address m_destination;
	uint8_t m_protocol;
	uint16_t m_identification;
};

class DefragStates {
public:
	DefragStates ();
	~DefragStates ();

	DefragState *lookup (ChunkIpv4 const *ip);
	void add (DefragState *state);
	void remove (DefragState *state);

private:
	typedef std::list <DefragState *> States;
	typedef std::list <DefragState *>::iterator StatesI;
	States m_states;
};

}; // namespace yans


#endif /* DEFRAG_STATE_H */
