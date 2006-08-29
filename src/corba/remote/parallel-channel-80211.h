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
#ifndef PARALLEL_CHANNEL_80211_H
#define PARALLEL_CHANNEL_80211_H

#include <list>
#include "yans/stdint.h"
#include "yans/base-channel-80211.h"
#include "remote-context.h"

namespace yans {
class PropagationModel;
class Packet;
};

class Channel80211Queue;

class ParallelChannel80211 : public yans::BaseChannel80211 {
public:
	ParallelChannel80211 ();
	virtual ~ParallelChannel80211 ();
	void add (Remote::Channel80211_var channel);
	void send_null_message (void);
	void receive (const ::Remote::SourcePosition& source_position, 
		      ::Remote::Timestamp source_time, 
		      const ::Remote::Buffer& buffer, 
		      CORBA::Double tx_power, 
		      CORBA::Octet tx_mode, 
		      CORBA::Octet stuff );
	void receive_null_message (::Remote::Timestamp ts, const ::Remote::SourcePositions& sources);
private:
	virtual void real_add (yans::PropagationModel *model);
	virtual void real_send (yans::Packet const *packet, double tx_power_dbm,
				uint8_t tx_mode, uint8_t stuff, 
				yans::PropagationModel const*caller) const;
	typedef std::list<yans::PropagationModel *> Models;
	typedef std::list<yans::PropagationModel *>::const_iterator ModelsCI;
	Models m_models;
	typedef std::list<Remote::Channel80211_var> Channels;
	typedef std::list<Remote::Channel80211_var>::iterator ChannelsI;
	typedef std::list<Remote::Channel80211_var>::const_iterator ChannelsCI;
	Channels m_channels;
	Channel80211Queue *m_queue;
};


#endif /* PARALLEL_CHANNEL_80211_H */
