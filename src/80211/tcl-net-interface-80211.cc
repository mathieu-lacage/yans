/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tcl-net-interface.h"
#include "net-interface-80211.h"
#include "tcl-tspec-request.h"
#include "tcl-tspec.h"
#include "mac-high.h"
#include "tcl-constructor.h"
#include "tcl-net-interface-80211.h"

TCL_CONSTRUCTOR (TclNetInterface80211, "TclNetInterface80211");
TCL_CONSTRUCTOR (TclNetInterfaceConstructor80211, "TclNetInterfaceConstructor80211");


TclNetInterface80211::TclNetInterface80211 () 
{}
TclNetInterface80211::~TclNetInterface80211 () 
{}

NetInterface *
TclNetInterface80211::getInterface (void)
{
	return m_interface;
}

void
TclNetInterface80211::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
}

int 
TclNetInterface80211::command(int argc, const char*const* argv)
{
	if (argc == 5) {
		if (strcmp (argv[1], "addts") == 0) {
			TclTspec *tclTspec = static_cast <TclTspec *> (lookup (argv[2]));
			TSpecRequest *request = new TclTSpecRequest (tclTspec->getTspec (), 
								     argv[2], argv[3], argv[4]);
			m_interface->high ()->addTsRequest (request);
			return TCL_OK;
		} else if (strcmp (argv[1], "delts") == 0) {
			TclTspec *tclTspec = static_cast <TclTspec *> (lookup (argv[2]));
			TSpecRequest *request = new TclTSpecRequest (tclTspec->getTspec (), 
								     argv[2], argv[3], argv[4]);
			m_interface->high ()->delTsRequest (request);
			return TCL_OK;
		}
	}
	return TclNetInterface::command (argc, argv);
}


TclNetInterfaceConstructor80211::TclNetInterfaceConstructor80211 ()
	: m_constructor (new NetInterfaceConstructor80211 ())
{}
TclNetInterfaceConstructor80211::~TclNetInterfaceConstructor80211 () 
{}

TclNetInterface *
TclNetInterfaceConstructor80211::createTclInterface (void)
{
	NetInterface80211 *interface;
	TclNetInterface80211 *tclInterface;
	interface = m_constructor->createInterface ();
	tclInterface = static_cast <TclNetInterface80211 *> (TclObject::New ("TclNetInterface80211"));
	tclInterface->setInterface (interface);
	return tclInterface;
}

int 
TclNetInterfaceConstructor80211::command(int argc, const char*const* argv)
{
	if (argc == 2) {
		if (strcmp (argv[1], "create-interface") == 0) {
			TclNetInterface *tclInterface = createTclInterface ();
			Tcl::instance ().result (tclInterface->name ());
			return TCL_OK;				
		} else if (strcmp (argv[1], "set-adhoc") == 0) {
			m_constructor->setAdhoc ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-qap") == 0) {
			m_constructor->setQap ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-nqap") == 0) {
			m_constructor->setNqap ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-arf") == 0) {
			m_constructor->setArf ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-aarf") == 0) {
			m_constructor->setAarf ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-80211a") == 0) {
			m_constructor->set80211a ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-free-space-propagation-model") == 0) {
			m_constructor->setFreeSpacePropagationModel ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-phy-model-ber") == 0) {
			m_constructor->setPhyModelBer ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-phy-model-snrt") == 0) {
			m_constructor->setPhyModelSnrt ();
			return TCL_OK;
		} else if (strcmp (argv[1], "set-phy-model-et") == 0) {
			m_constructor->setPhyModelEt ();
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcmp (argv[1], "set-qsta") == 0) {
			int apAddress = atoi (argv[2]);
			m_constructor->setQsta (apAddress);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-nqsta") == 0) {
			int apAddress = atoi (argv[2]);
			m_constructor->setNqsta (apAddress);
			return TCL_OK;
		}
	} else if (argc == 4) {
		if (strcmp (argv[1], "set-cr") == 0) {
			int dataMode = atoi (argv[2]);
			int ctlMode = atoi (argv[3]);
			m_constructor->setCr (dataMode, ctlMode);
			return TCL_OK;
		}
	}
	return TclNetInterfaceConstructor::command (argc, argv);
}
