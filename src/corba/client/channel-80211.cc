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
#include "channel-80211.h"
#include "network-interface-80211.h"
#include "id-factory.h"

namespace yapns {

Channel80211::Channel80211 ()
{}

Channel80211::~Channel80211 ()
{
	for (ContextsI i = m_contexts.begin (); i != m_contexts.end (); i++) {
		CORBA::release (i->second);
	}
	m_contexts.erase (m_contexts.begin (), m_contexts.end ());
}

void 
Channel80211::record_connect (NetworkInterface80211 *interface)
{
	SimulationContext ctx = interface->get_context ();
	::Remote::NetworkInterface80211_ptr remote_80211 = interface->get_remote_80211 ();
	for (ContextsI i = m_contexts.begin (); i != m_contexts.end (); i++) {
		if (i->first->is_equal (ctx)) {
			remote_80211->connect (i->second);
			goto out;
		}
	}
	::Remote::Channel80211_ptr remote_channel = ctx->peek_remote ()->create_channel_80211 (IdFactory::get_next ());
	for (ContextsI j = m_contexts.begin (); j != m_contexts.end (); j++) {
		remote_channel->add (j->second);
		j->second->add (remote_channel);
	}
	m_contexts.push_back (std::make_pair (ctx, remote_channel));
	remote_80211->connect (remote_channel);
 out:
	CORBA::release (remote_80211);
}

}; // namespace yapns
