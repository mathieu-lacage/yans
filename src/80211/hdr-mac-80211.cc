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
hdr_mac_80211::getDestination (void) const
{
	return m_destination;
}
int 
hdr_mac_80211::getFinalDestination (void) const
{
	return m_finalDestination;
}
int 
hdr_mac_80211::getSource (void) const
{
	return m_source;
}
enum mac_80211_data_type
hdr_mac_80211::getDataType (void) const
{
	return m_dataType;
}
enum mac_80211_packet_type 
hdr_mac_80211::getType (void) const
{
	return m_type;
}
double 
hdr_mac_80211::getDuration (void) const
{
	return m_duration;
}
int 
hdr_mac_80211::getSequence (void) const
{
	return m_sequence;
}
bool
hdr_mac_80211::isRetry (void) const
{
	return m_retry;
}

void 
hdr_mac_80211::setDestination (int destination)
{
	m_destination = destination;
}
void 
hdr_mac_80211::setFinalDestination (int destination)
{
	m_finalDestination = destination;
}
void 
hdr_mac_80211::setSource (int source)
{
	m_source = source;
}
void 
hdr_mac_80211::setDataType (enum mac_80211_data_type dataType)
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
hdr_mac_80211::getTxMode (void) const
{
	return m_txMode;
}

#define FOO(x) case MAC_80211_ ## x: return #x; break;

const char *
hdr_mac_80211::getDataTypeString (void) const
{
	switch (m_dataType) {
		FOO (BEACON);
		FOO (ASSOCIATTION_REQUEST);
		FOO (ASSOCIATTION_RESPONSE);
		FOO (DISASSOCIATION);
		FOO (REASSOCIATION_REQUEST);
		FOO (REASSOCIATION_RESPONSE);
		FOO (PROBE_REQUEST);
		FOO (PROBE_RESPONSE);
		FOO (AUTHENTICATION);
		FOO (DEAUTHENTICATION);
	}
	return "deadbeaf";
}
const char *
hdr_mac_80211::getTypeString (void) const
{
	switch (m_type) {
		FOO (RTS);
		FOO (CTS);
		FOO (DATA);
		FOO (ACK);
	}
	return "deadbeaf";
}

#undef FOO


int 
getDestination (Packet *packet)
{
	int destination = HDR_MAC_80211 (packet)->getDestination ();
	return destination;
}
int 
getFinalDestination (Packet *packet)
{
	int destination = HDR_MAC_80211 (packet)->getFinalDestination ();
	return destination;
}
int 
getSource (Packet *packet)
{
	int source = HDR_MAC_80211 (packet)->getSource ();
	return source;
}
int 
getTxMode (Packet *packet)
{
	int txMode = HDR_MAC_80211 (packet)->getTxMode ();
	return txMode;
}
enum mac_80211_packet_type
getType (Packet *packet)
{
	enum mac_80211_packet_type type;
	type = HDR_MAC_80211 (packet)->getType ();
	return type;
}
bool
isRetry (Packet *packet)
{
	bool isRetry = HDR_MAC_80211 (packet)->isRetry ();
	return isRetry;
}
int
getSequence (Packet *packet)
{
	int sequence = HDR_MAC_80211 (packet)->getSequence ();
	return sequence;
}
int
getSize (Packet *packet)
{
	return HDR_CMN (packet)->size ();
}



double
getDuration (Packet *packet)
{
	return HDR_MAC_80211 (packet)->getDuration ();
}



void
setSize (Packet *packet, int size)
{
	HDR_CMN (packet)->size () = size;
}
void
increaseSize (Packet *packet, int increment)
{
	HDR_CMN (packet)->size () += increment;
	//cout << "increase " << packet << " by " << increment << endl;
}
void
decreaseSize (Packet *packet, int decrement)
{
	HDR_CMN (packet)->size () -= decrement;
	//cout << "decrease " << packet << " by " << decrement << endl;
}
void
setTxMode (Packet *packet, int mode)
{
	HDR_MAC_80211 (packet)->setTxMode (mode);
}
void
setDestination (Packet *packet, int destination)
{
	HDR_MAC_80211 (packet)->setDestination (destination);
}
void
setDuration (Packet *packet, double duration)
{
	HDR_MAC_80211 (packet)->setDuration (duration);
}
void
setSource (Packet *packet, int source)
{
	HDR_MAC_80211 (packet)->setSource (source);
}
void
setFinalDestination (Packet *packet, int destination)
{
	HDR_MAC_80211 (packet)->setFinalDestination (destination);
}
void
setType (Packet *packet, enum mac_80211_packet_type type)
{
	HDR_MAC_80211 (packet)->setType (type);
}
void
setSequence (Packet *packet, int sequence)
{
	HDR_MAC_80211 (packet)->setSequence (sequence);
}
void
setRetry (Packet *packet)
{
	HDR_MAC_80211 (packet)->setRetry ();
}
void
initialize (Packet *packet)
{
	HDR_MAC_80211 (packet)->initialize ();
}
