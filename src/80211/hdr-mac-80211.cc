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

#include "hdr-mac-80211.h"
#include "packet.h"
#include "mac.h"

/* a small array to verify that 
 * sizeof (hdr_mac) >= sizeof (hdr_mac_80211) 
 */
static char foo[sizeof (hdr_mac)-sizeof (hdr_mac_80211)+1];

void
hdr_mac_80211::initialize (void)
{
	m_retry = false;
}

int 
hdr_mac_80211::getDestination (void)
{
	return m_destination;
}
int 
hdr_mac_80211::getSource (void)
{
	return m_source;
}
uint16_t 
hdr_mac_80211::getDataType (void)
{
	return m_dataType;
}
enum mac_80211_packet_type 
hdr_mac_80211::getType (void)
{
	return m_type;
}
double 
hdr_mac_80211::getDuration (void)
{
	return m_duration;
}
int 
hdr_mac_80211::getSequence (void)
{
	return m_sequence;
}
bool
hdr_mac_80211::isRetry (void)
{
	return m_retry;
}

void 
hdr_mac_80211::setDestination (int destination)
{
	m_destination = destination;
}
void 
hdr_mac_80211::setSource (int source)
{
	m_source = source;
}
void 
hdr_mac_80211::setDataType (uint16_t dataType)
{
	m_dataType = dataType;
}
void 
hdr_mac_80211::setType (enum mac_80211_packet_type type)
{
	m_type = type;
}
void 
hdr_mac_80211::setDuration (double duration)
{
	m_duration = duration;
}
void 
hdr_mac_80211::setSequence (int sequence)
{
	m_sequence = sequence;
}
void
hdr_mac_80211::setRetry (void)
{
	m_retry = true;
}

void
hdr_mac_80211::setTxMode (int mode)
{
	m_txMode = mode;
}

int
hdr_mac_80211::getTxMode (void)
{
	return m_txMode;
}
