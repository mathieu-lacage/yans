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
#include "base-channel-80211.h"

namespace yans {

BaseChannel80211::~BaseChannel80211 ()
{}
void 
BaseChannel80211::add (PropagationModel *model)
{
	real_add (model);
}
void 
BaseChannel80211::send (Packet const packet, double tx_power_dbm,
			uint8_t tx_mode, uint8_t stuff, 
			PropagationModel const*caller) const
{
	real_send (packet, tx_power_dbm,
		   tx_mode, stuff, caller);
}

}; // namespace yans
