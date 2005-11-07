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

class Packet;

class DefragState {
public:
	DefragState ();
	~DefragState ();

	void add (Packet *fragment);
	bool is_complete (void);
	Packet *get_complete (void);

	bool is_too_old (void);
	bool matches (Packet *fragment);
	Ipv4Address get_source (void);
	Ipv4Address get_destination (void);
	uint8_t get_protocol (void);
	uint16_t get_identification (void);

private:
	double get_reassembly_timeout (void);

	typedef std::list <Packet *> Fragments;
	typedef std::list <Packet *>::iterator FragmentsI;

	double m_defrag_start;
	Fragments m_fragments;
};

class DefragStates {
public:
	DefragStates ();
	~DefragStates ();

	DefragState *lookup (Packet *packet);
	void add (DefragState *state);
	void remove (DefragState *state);

private:
	typedef std::list <DefragState *> States;
	typedef std::list <DefragState *>::iterator StatesI;
	States m_states;
};


#endif /* DEFRAG_STATE_H */
