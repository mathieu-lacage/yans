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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */
#include "channel-80211.h"
#include "packet.h"
#include "propagation-model.h"

namespace yans {

Channel80211::Channel80211 ()
{}
Channel80211::~Channel80211 ()
{}
void 
Channel80211::add (PropagationModel *model)
{
	m_models.push_back (model);
}
void 
Channel80211::send (Packet const *packet, PropagationData const *data, 
		    uint8_t tx_mode, uint8_t stuff,
		    PropagationModel const *caller) const
{
	for (ModelsCI i = m_models.begin (); i != m_models.end (); i++) {
		if (caller != (*i)) {
			(*i)->receive (packet, data, tx_mode, stuff);
		}
	}
}

}; // namespace yans
