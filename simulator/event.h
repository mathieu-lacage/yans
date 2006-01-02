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

#ifndef EVENT_H
#define EVENT_H

namespace yans {

class Event {
public:
	/* Events do not have a destructor on purpose. The notify method
	 * below is supposed to do whatever is needed to destroy event.
	 * Of course, it can be called at most once. It is possible
	 * for an event's notify method never to be called even if it
	 * was scheduled in the simulator. This happens only if the user
	 * of the simulation interrupts the simulation before its natural
	 * completion through a call to Simulator::stop ().
	 * Natural completion of a simulation should happen when there are
	 * no events to serve anymore.
	 */
	virtual void notify (void) = 0;
	/* Actually, we declare a virtual destructor just to quiet the warnings
	 * generated by gcc.
	 */
	virtual ~Event ();
};

}; // namespace yans

#include "event.tcc"

#endif /* EVENT_H */