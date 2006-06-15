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
#include "throughput-printer.h"

namespace yapns {


ThroughputPrinter::ThroughputPrinter (SimulationContext ctx)
{
	m_remote = ctx->peek_remote ()->create_throughput_printer ();
}

ThroughputPrinter::~ThroughputPrinter ()
{
	CORBA::release (m_remote);
}
void 
ThroughputPrinter::receive (Packet *packet)
{
	assert (false);
}

::Remote::ThroughputPrinter_ptr 
ThroughputPrinter::peek_remote (void)
{
	return m_remote;
}


}; // namespace yapns
