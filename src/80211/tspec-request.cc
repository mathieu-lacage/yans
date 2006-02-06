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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tspec-request.h"
#include "tspec.h"

TSpecRequest::TSpecRequest (TSpec *tspec)
	: m_tspec (tspec)
{}
TSpecRequest::~TSpecRequest ()
{}

TSpec *
TSpecRequest::getTSpec (void)
{
	return m_tspec;
}

void 
TSpecRequest::notifyGranted (void)
{
	notifyRequestGranted ();
}
void 
TSpecRequest::notifyRefused (void)
{
	notifyRequestRefused ();
}


void TSpecRequest::notifyRequestGranted (void)
{}
void TSpecRequest::notifyRequestRefused (void)
{}


TclTSpecRequest::TclTSpecRequest (TSpec *tspec, char const *tclTspec,
				  char const *grantedCallback,
				  char const *refusedCallback)
	: TSpecRequest (tspec),
	  m_tclTspec (tclTspec),
	  m_grantedCallback (strdup (grantedCallback)),
	  m_refusedCallback (strdup (refusedCallback))
{}
TclTSpecRequest::~TclTSpecRequest ()
{
	delete m_tclTspec;
	delete m_grantedCallback;
	delete m_refusedCallback;
}

void 
TclTSpecRequest::notifyRequestGranted (void)
{
	Tcl& tcl=Tcl::instance();
	tcl.evalf ("%s %s %d", m_grantedCallback, m_tclTspec, getTSpec ()->getTSID ());
}
void 
TclTSpecRequest::notifyRequestRefused (void)
{
	Tcl& tcl=Tcl::instance();
	tcl.evalf ("%s %s %d", m_refusedCallback, m_tclTspec, getTSpec ()->getTSID ());
}

