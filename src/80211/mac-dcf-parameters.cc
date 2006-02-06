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

#include <math.h>
#include <cassert>

#include "mac-dcf-parameters.h"
#include "mac-parameters.h"
#include "phy-80211.h"

#define nopeMAC_DEBUG 1

#ifdef MAC_DEBUG
# define DEBUG(format, ...) \
	printf ("DEBUG " format "\n", ## __VA_ARGS__);
#else /* MAC_DEBUG */
# define DEBUG(format, ...)
#endif /* MAC_DEBUG */


MacDcfParameters::MacDcfParameters ()
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

void 
MacDcfParameters::setParameters (MacParameters *parameters)
{
	m_parameters = parameters;
}

double 
MacDcfParameters::getAIFS (void)
{
	return m_parameters->getSIFS () + 
		m_AIFSN * m_parameters->getSlotTime ();
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
MacDcfParameters::getEIFS (Phy80211 *phy)
{
	/* 802.11 section 9.2.10 */
	// XXX check with regard to 802.11a
	return m_parameters->getSIFS () + 
		phy->calculateTxDuration (0, m_parameters->getACKSize ()) +
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
MacDcfParameters::setACM (bool enabled)
{
	m_ACM = enabled;
}

uint8_t
MacDcfParameters::log2 (uint16_t v)
{
	uint8_t log = 0;
	do {
		log++;
	} while (v >>= 1);
	return log;
}

void
MacDcfParameters::writeTo (uint8_t buffer[4], enum ac_e ac)
{
	uint8_t ACM = ((m_ACM)?1:0) << 3;
	uint8_t AIFSN = m_AIFSN << 4;
	uint8_t ACI;
	switch (ac) {
	case AC_BE:
		ACI = 0;
		break;
	case AC_BK:
		ACI = 1;
		break;
	case AC_VI:
		ACI = 2;
		break;
	case AC_VO:
		ACI = 3;
		break;
	default:
		/* quiet compiler warning */
		ACI = 4;
		assert (false);
		break;
	}
	ACI <<= 1;
	buffer[0] = AIFSN | ACM | ACI;

	uint8_t ECWmin = log2 (m_CWmin) << 4;
	uint8_t ECWmax = log2 (m_CWmax);
	buffer[1] = ECWmin | ECWmax;

	long int txopLimit = lrint (m_txopLimit / 32e-6);
	buffer[2] = (txopLimit >> 8) & 0xff;
	buffer[3] = (txopLimit >> 0) & 0xff;
}

void
MacDcfParameters::readFrom (uint8_t const buffer[4])
{
	uint8_t AIFSN = (buffer[0] >> 4) & 0x0f;
	uint8_t ACM   = (buffer[0] >> 3) & 0x01;
	//uint8_t ACI   = (buffer[0] >> 1) & 0x03;
	uint8_t ECWmin = (buffer[1] >> 4) & 0x0f;
	uint8_t ECWmax = (buffer[1] >> 0) & 0x0f;
	uint16_t txopLimit = (buffer[2] << 8) | buffer[3];

	m_AIFSN = AIFSN;
	m_ACM = (ACM == 1)?true:false;
	m_CWmin = (1<<ECWmin) - 1;
	m_CWmax = (1<<ECWmax) - 1;
	m_txopLimit = txopLimit * 32e-6;
}
