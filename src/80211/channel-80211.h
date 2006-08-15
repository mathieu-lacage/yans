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
#ifndef CHANNEL_80211_H
#define CHANNEL_80211_H

#include <list>
#include <stdint.h>
#include "base-channel-80211.h"
#include "packet.h"

namespace yans {

class PropagationModel;

class Channel80211 : public BaseChannel80211 {
public:
	Channel80211 ();
	virtual ~Channel80211 ();
private:
	virtual void real_add (PropagationModel *model);
	virtual void real_send (Packet const packet, double tx_power_dbm,
				uint8_t tx_mode, uint8_t stuff, 
				PropagationModel const*caller) const;
	typedef std::list<PropagationModel *> Models;
	typedef std::list<PropagationModel *>::const_iterator ModelsCI;
	Models m_models;
};

}; // namespace yans


#endif /* CHANNEL_80211_H */
