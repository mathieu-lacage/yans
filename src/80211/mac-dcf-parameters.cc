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
	/* AIFS = DIFS: 802.11 section 9.2.10 */
	m_AIFSN = 2;
	/* XXX 802.11a */
	m_CWmin = 15;
	m_CWmax = 1023;
	m_txopLimit = 0.0;

	DEBUG ("DIFS %f", getAIFS ());
	DEBUG ("EIFS %f", getEIFS ());
	DEBUG ("CWmin %d", getCWmin ());
	DEBUG ("CWmax %d", getCWmax ());
}

double 
MacDcfParameters::getAIFS (void)
{
	return m_container->parameters ()->getSIFS () + 
		m_AIFSN * m_container->parameters ()->getSlotTime ();
}
int
MacDcfParameters::getCWmin (void)
{
	return m_CWmin;
}
int
MacDcfParameters::getCWmax (void)
{
	return m_CWmax;
}
double 
MacDcfParameters::getTxopLimit (void)
{
	return m_txopLimit;
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


bool 
MacDcfParameters::isACMandatory (void)
{
	return m_ACM;
}

void 
MacDcfParameters::setCWmin (uint16_t CWmin)
{
	m_CWmin = CWmin;
}
void 
MacDcfParameters::setCWmax (uint16_t CWmax)
{
	m_CWmax = CWmax;
}
void 
MacDcfParameters::setTxopLimit (double txopLimit)
{
	m_txopLimit = txopLimit;
}
void 
MacDcfParameters::setAIFSN (uint8_t AIFSN)
{
	m_AIFSN = AIFSN;
}
void 
MacDcfParameters::setACM (uint8_t ACM)
{
	m_ACM = (ACM == 1)?true:false;
}
