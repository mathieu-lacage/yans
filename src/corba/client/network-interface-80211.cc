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
#include "network-interface-80211.h"
#include "channel-80211.h"

namespace yapns {

NetworkInterface80211::NetworkInterface80211 (SimulationContext ctx)
	: m_context (ctx)
{}
NetworkInterface80211::~NetworkInterface80211 ()
{}

::Remote::NetworkInterface80211_ptr 
NetworkInterface80211::get_remote_80211 (void)
{
	return real_get_remote_80211 ();
}

SimulationContext
NetworkInterface80211::get_context (void)
{
	return m_context;
}


void 
NetworkInterface80211::connect_to (Channel80211 *channel)
{
	channel->record_connect (this);
}


NetworkInterface80211Adhoc::NetworkInterface80211Adhoc (SimulationContext ctx, ::Remote::NetworkInterface80211Adhoc_ptr remote)
	: NetworkInterface80211 (ctx)
{
	m_remote = ::Remote::NetworkInterface80211Adhoc::_duplicate (remote);
}
NetworkInterface80211Adhoc::~NetworkInterface80211Adhoc ()
{
	CORBA::release (m_remote);
}

::Remote::MacNetworkInterface_ptr 
NetworkInterface80211Adhoc::real_get_remote (void)
{
	return ::Remote::MacNetworkInterface::_narrow (m_remote);
}
::Remote::NetworkInterface80211_ptr 
NetworkInterface80211Adhoc::real_get_remote_80211 (void)
{
	return ::Remote::NetworkInterface80211::_narrow (m_remote);
}



};
