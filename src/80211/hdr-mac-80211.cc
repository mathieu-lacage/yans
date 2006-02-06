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
	m_retry = 0;
	m_moreFragments = 0;
	m_sequenceControl = 0;
	m_ackMode = ACK_NORMAL;
	m_qos = 0;
}


/*********************************************************
 * Getters.
 ********************************************************/

int
hdr_mac_80211::getTxMode (void) const
{
	return m_txMode;
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
uint16_t
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
	double realDuration = (double)m_duration;
	return realDuration / 1e6;
}
int 
hdr_mac_80211::getSequenceControl (void) const
{
	return m_sequenceControl;
}
bool
hdr_mac_80211::getMoreFragments (void) const
{
	return (m_moreFragments)?true:false;
}
bool
hdr_mac_80211::isRetry (void) const
{
	return (m_retry)?true:false;
}
int 
hdr_mac_80211::getTID (void) const
{
	return m_tid;
}
bool 
hdr_mac_80211::isBlockAck (void) const
{
	return (m_ackMode == ACK_BLOCK)?true:false;
}
bool 
hdr_mac_80211::isNoAck (void) const 
{
	return (m_ackMode == ACK_NO)?true:false;
}
bool 
hdr_mac_80211::isNormalAck (void) const
{
	return (m_ackMode == ACK_NORMAL)?true:false;
}
bool 
hdr_mac_80211::isQos (void) const
{
	return (m_qos)?true:false;
}

/*********************************************************
 * Setters.
 ********************************************************/

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
	if (fabs (duration) < 1e6) {
		duration = 0.0;
	}
	assert (duration >= 0);
	unsigned int realDuration = ( unsigned int) floor (duration * 1e6 + 0.5);
	m_duration = realDuration;
}
void 
hdr_mac_80211::setSequenceNumber (int sequence)
{
	m_sequenceControl |= (sequence << 4);
}
void 
hdr_mac_80211::setFragmentNumber (int fragmentNumber)
{
	m_sequenceControl |= fragmentNumber & 0x0f;
}
void
hdr_mac_80211::setRetry (void)
{
	m_retry = 1;
}

void
hdr_mac_80211::setTxMode (int mode)
{
	m_txMode = mode;
}
void
hdr_mac_80211::setMoreFragments (bool moreFragments)
{
	m_moreFragments = moreFragments?1:0;
}
void 
hdr_mac_80211::setTID (int tid)
{
	m_tid = tid;
	m_qos = 1;
}
void 
hdr_mac_80211::setNoAck (void)
{
	m_ackMode = ACK_NO;
}
void 
hdr_mac_80211::setBlockAck (void) 
{
	m_ackMode = ACK_BLOCK;
}
void 
hdr_mac_80211::setNormalAck (void)
{
	m_ackMode = ACK_NORMAL;
}


/*********************************************************
 * enum to string conversion.
 ********************************************************/


#define FOO(x) case MAC_80211_ ## x: return #x; break;

char const *
hdr_mac_80211::getTypeString (void) const
{
	switch (m_type) {
		FOO (CTL_RTS);
		FOO (CTL_CTS);
		FOO (CTL_ACK);
		FOO (CTL_BACKREQ);
		FOO (CTL_BACKRESP);
		FOO (DATA);
		FOO (MGT_BEACON);
		FOO (MGT_ASSOCIATION_REQUEST);
		FOO (MGT_ASSOCIATION_RESPONSE);
		FOO (MGT_DISASSOCIATION);
		FOO (MGT_REASSOCIATION_REQUEST);
		FOO (MGT_REASSOCIATION_RESPONSE);
		FOO (MGT_PROBE_REQUEST);
		FOO (MGT_PROBE_RESPONSE);
		FOO (MGT_AUTHENTICATION);
		FOO (MGT_DEAUTHENTICATION);
		FOO (MGT_ADDBA_REQUEST);
		FOO (MGT_ADDBA_RESPONSE);
		FOO (MGT_DELBA_REQUEST);
		FOO (MGT_DELBA_RESPONSE);
	}
	return "deadbeaf";
}

#undef FOO


/*********************************************************
 * Helper Getters
 ********************************************************/


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
getSequenceControl (Packet *packet)
{
	int sequence = HDR_MAC_80211 (packet)->getSequenceControl ();
	return sequence;
}
bool
getMoreFragments (Packet *packet)
{
	bool moreFragments = HDR_MAC_80211 (packet)->getMoreFragments ();
	return moreFragments;
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
int 
getTID (Packet *packet)
{
	return HDR_MAC_80211 (packet)->getTID ();
}
bool 
isBlockAck (Packet *packet)
{
	return HDR_MAC_80211 (packet)->isBlockAck ();
}
bool 
isNoAck (Packet *packet)
{
	return HDR_MAC_80211 (packet)->isNoAck ();
}
bool 
isNormalAck (Packet *packet)
{
	return HDR_MAC_80211 (packet)->isNormalAck ();
}
bool 
isQos (Packet *packet)
{
	return HDR_MAC_80211 (packet)->isQos ();
}



/*********************************************************
 * Helper Setters
 ********************************************************/


void 
setTID (Packet *packet, int tid)
{
	HDR_MAC_80211 (packet)->setTID (tid);
}
void 
setNoAck (Packet *packet)
{
	HDR_MAC_80211 (packet)->setNoAck ();
}
void 
setBlockAck (Packet *packet)
{
	HDR_MAC_80211 (packet)->setBlockAck ();
}
void
setNormalAck (Packet *packet)
{
	HDR_MAC_80211 (packet)->setNormalAck ();
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
setSequenceNumber (Packet *packet, int sequence)
{
	HDR_MAC_80211 (packet)->setSequenceNumber (sequence);
}
void
setFragmentNumber (Packet *packet, int fragmentNumber)
{
	HDR_MAC_80211 (packet)->setFragmentNumber (fragmentNumber);
}
void
setRetry (Packet *packet)
{
	HDR_MAC_80211 (packet)->setRetry ();
}
void
setMoreFragments (Packet *packet, bool moreFragments)
{
	HDR_MAC_80211 (packet)->setMoreFragments (moreFragments);
}
void
initialize (Packet *packet)
{
	HDR_MAC_80211 (packet)->initialize ();
}








char const *
getTypeString (Packet *packet)
{
	return HDR_MAC_80211 (packet)->getTypeString ();
}
