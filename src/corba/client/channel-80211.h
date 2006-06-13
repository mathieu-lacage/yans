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
#ifndef YAPNS_CHANNEL_80211_H
#define YAPNS_CHANNEL_80211_H

#include "simulation-context.h"
#include "remote-context.h"
#include <vector>
#include <utility>

namespace yapns {

class NetworkInterface80211;

class Channel80211 {
public:
	Channel80211 ();
	~Channel80211 ();
	void record_connect (NetworkInterface80211 *interface);
private:
	typedef std::vector<std::pair<SimulationContext,::Remote::Channel80211_ptr> > Contexts;
	typedef std::vector<std::pair<SimulationContext,::Remote::Channel80211_ptr> >::iterator ContextsI;

	Contexts m_contexts;
};

}; // namespace yapns

#endif /* YAPNS_CHANNEL_80211_H */
