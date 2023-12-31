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
	m__aifsn = 2;
	/* XXX 802.11a */
	m__cwmin = 15;
	m__cwmax = 1023;
	m_txop_limit = 0.0;

	DEBUG ("DIFS %f", get_aifs ());
	DEBUG ("EIFS %f", get_eifs ());
	DEBUG ("CWmin %d", get_cwmin ());
	DEBUG ("CWmax %d", get_cwmax ());
}

void 
MacDcfParameters::set_parameters (MacParameters *parameters)
{
	m_parameters = parameters;
}

double 
MacDcfParameters::get_aifs (void)
{
	return m_parameters->getSIFS () + 
		m_AIFSN * m_parameters->getSlotTime ();
}
int
MacDcfParameters::get_cwmin (void)
{
	return m_CWmin;
}
int
MacDcfParameters::get_cwmax (void)
{
	return m_CWmax;
}
double 
MacDcfParameters::get_txop_limit (void)
{
	return m_txopLimit;
}

double 
MacDcfParameters::get_eifs (Phy80211 *phy)
{
	/* 802.11 section 9.2.10 */
	// XXX check with regard to 802.11a
	return m_parameters->getSIFS () + 
		phy->calculateTxDuration (0, m_parameters->get_acksize ()) +
		getAIFS ();
}


bool 
MacDcfParameters::is_acmandatory (void)
{
	return m_ACM;
}

void 
MacDcfParameters::set_cwmin (uint16_t CWmin)
{
	m__cwmin = CWmin;
}
void 
MacDcfParameters::set_cwmax (uint16_t CWmax)
{
	m__cwmax = CWmax;
}
void 
MacDcfParameters::set_txop_limit (double txopLimit)
{
	m_txop_limit = txopLimit;
}
void 
MacDcfParameters::set_aifsn (uint8_t AIFSN)
{
	m__aifsn = AIFSN;
}

void 
MacDcfParameters::set_acm (bool enabled)
{
	m__acm = enabled;
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
MacDcfParameters::write_to (uint8_t buffer[4], enum ac_e ac)
{
	uint8_t ACM = ((m_ACM)?1:0) << 3;
	uint8_t AIFSN = m__aifsn << 4;
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

	long int txopLimit = lrint (m_txop_limit / 32e-6);
	buffer[2] = (txopLimit >> 8) & 0xff;
	buffer[3] = (txopLimit >> 0) & 0xff;
}

void
MacDcfParameters::read_from (uint8_t const buffer[4])
{
	uint8_t AIFSN = (buffer[0] >> 4) & 0x0f;
	uint8_t ACM   = (buffer[0] >> 3) & 0x01;
	//uint8_t ACI   = (buffer[0] >> 1) & 0x03;
	uint8_t ECWmin = (buffer[1] >> 4) & 0x0f;
	uint8_t ECWmax = (buffer[1] >> 0) & 0x0f;
	uint16_t txopLimit = (buffer[2] << 8) | buffer[3];

	m__aifsn = AIFSN;
	m__acm = (ACM == 1)?true:false;
	m__cwmin = (1<<ECWmin) - 1;
	m__cwmax = (1<<ECWmax) - 1;
	m_txop_limit = txopLimit * 32e-6;
}
