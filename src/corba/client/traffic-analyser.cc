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
#include "traffic-analyser.h"

namespace yapns {


TrafficAnalyser::TrafficAnalyser (SimulationContext ctx)
{
	m_remote = ctx->peek_remote ()->create_traffic_analyser ();
}

TrafficAnalyser::~TrafficAnalyser ()
{
	CORBA::release (m_remote);
}
void 
TrafficAnalyser::receive (Packet *packet)
{
	assert (false);
}

::Remote::TrafficAnalyser_ptr 
TrafficAnalyser::peek_remote (void)
{
	return m_remote;
}


}; // namespace yapns
