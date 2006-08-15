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
#ifndef BASE_CHANNEL_80211_H
#define BASE_CHANNEL_80211_H

#include <list>
#include <stdint.h>
#include "gpacket.h"

namespace yans {

class PropagationModel;

class BaseChannel80211 {
public:
	virtual ~BaseChannel80211 () = 0;
	void add (PropagationModel *model);
	void send (Packet const packet, double tx_power_dbm,
		   uint8_t tx_mode, uint8_t stuff, 
		   PropagationModel const*caller) const;
private:
	virtual void real_add (PropagationModel *model) = 0;
	virtual void real_send (Packet const packet, double tx_power_dbm,
				uint8_t tx_mode, uint8_t stuff, 
				PropagationModel const*caller) const = 0;
};

}; // namespace yans


#endif /* BASE_CHANNEL_80211_H */
