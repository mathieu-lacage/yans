/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#include "mac-station.h"
#include "mac-stations.h"

namespace yans {

MacStation::MacStation ()
	: m_state (DISASSOC)
{}


MacStation::~MacStation ()
{}

bool 
MacStation::is_associated (void) const
{
	return m_state == GOT_ASSOC_TX_OK;
}
bool 
MacStation::is_wait_assoc_tx_ok (void) const
{
	return m_state == WAIT_ASSOC_TX_OK;
}
void 
MacStation::record_wait_assoc_tx_ok (void)
{
	m_state = WAIT_ASSOC_TX_OK;
}
void 
MacStation::record_got_assoc_tx_ok (void)
{
	m_state = GOT_ASSOC_TX_OK;
}
void 
MacStation::record_got_assoc_tx_failed (void)
{
	m_state = DISASSOC;
}
void 
MacStation::record_disassociated (void)
{
	m_state = DISASSOC;
}

}; // namespace yans
