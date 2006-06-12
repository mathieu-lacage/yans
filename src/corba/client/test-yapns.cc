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
#include "yapns/host.h"
#include "yapns/simulation-context.h"
#include "yapns/simulator.h"
#include "yapns/mac-address-factory.h"
#include "yapns/network-interface-80211-factory.h"
#include "yapns/network-interface-80211.h"
#include "yapns/static-position.h"

using namespace yapns;


int main (int argc, char *argv[])
{
	SimulationContextFactory ctx;
	ctx.read_configuration ("sample.xml");
	NetworkInterface80211Factory factory;
	MacAddressFactory address_factory;

	SimulationContext a_context = ctx.lookup ("a");
	Host *a = new Host (a_context, "a");
	Position *pos_a = new StaticPosition (a_context);
	NetworkInterface80211Adhoc *interface_a = factory.create_adhoc (a_context, 
									address_factory.get_next (), 
									pos_a);
	


	Host *b = new Host (ctx.lookup ("b"), "b");
	

	Simulator::run ();

	delete a;
	Simulator::destroy ();

	return 0;
}
