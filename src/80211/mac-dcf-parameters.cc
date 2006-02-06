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

#include "mac-dcf-parameters.h"
#include "mac-container.h"
#include "mac-parameters.h"
#include "phy-80211.h"

#define nopeMAC_DEBUG 1

#ifdef MAC_DEBUG
# define DEBUG(format, ...) \
	printf ("DEBUG " format "\n", ## __VA_ARGS__);
#else /* MAC_DEBUG */
# define DEBUG(format, ...)
#endif /* MAC_DEBUG */


MacDcfParameters::MacDcfParameters (MacContainer *container)
	: m_container (container)
{
	DEBUG ("DIFS %f", getAIFS ());
	DEBUG ("EIFS %f", getEIFS ());
	DEBUG ("CWmin %d", getCWmin ());
	DEBUG ("CWmax %d", getCWmax ());
}

double 
MacDcfParameters::getAIFS (void)
{
	/* 802.11 section 9.2.10 */
	return m_container->parameters ()->getSIFS () + 
		2 * m_container->parameters ()->getSlotTime ();
}
int
MacDcfParameters::getCWmin (void)
{
	/* XXX 802.11a */
	return 15;
}
int
MacDcfParameters::getCWmax (void)
{
	/* XXX 802.11a */
	return 1023;
}
double 
MacDcfParameters::getTxopLimit (void)
{
	return 0;
}

double 
MacDcfParameters::getEIFS (void)
{
	/* 802.11 section 9.2.10 */
	// XXX check with regard to 802.11a
	return m_container->parameters ()->getSIFS () + 
		m_container->phy ()->calculateTxDuration (0, m_container->parameters ()->getACKSize ()) +
		getAIFS ();
}
