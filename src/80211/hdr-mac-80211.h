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
#ifndef HDRMAC_80211
#define HDRMAC_80211

/* Using this code is easy:
 *  - call HDR_MAC_80211 (packet)->foo () as you wish.
 *  - assume that the higher-levels set and get the
 *    fields source, destination and dataType.
 *
 * To understand why it works is clearly a bit more tricky.
 * There are quite a few tricks here so I think it is
 * worth talking about them. Theorically, if you want to 
 * introduce a new packet type, you need to declare a new 
 * header class/structure which features a static 
 * int _offset member and you need to implement a static
 * instance of a subclass of PacketHeaderClass.
 * This static instance should register this new header type
 * and store in the static _offset member the offset at which
 * the header can be accessed by a Packet's access method.
 *
 * This is the theory. Unfortunately, it is not really 
 * possible to create a new packet type and have other pieces
 * of the ns2 codebase start using it even though it provides
 * at least the same functionality. Specifically, you cannot
 * create a new Mac-like packet type to replace the hdr_mac 
 * headers because lots of places access directly to the 
 * hdr_mac header through the HDR_MAC macro. In theory, it 
 * would be possible to remove this dependency on the HDR_MAC 
 * macro (through the introduction of decent methods in the 
 * Mac class such as: set_dst (Packet *, int) and the removal
 * of methods such as hdr_dst (char *, int = -2)) _but_ I have
 * decided to make as little modifications as possible to the 
 * core of ns2 which means that I cannot really do this. 
 * As such, I have ended-up using the same trick used in the
 * previous 802.11 mac which is to use the hdr_mac structure
 * to store my 802.11 header.
 * 
 * For example:
 *   - HDR_MAC_80211: calls hdr_mac::access rather than 
 *     hdr_mac_80211::access.
 *   - class hdr_mac_80211 does not have a static int 
 *     _offset member.
 *   - I do not declare a subclass of PacketHeaderClass.
 *
 * All of this is going to work provided that 
 * sizeof (hdr_mac) >= sizeof (hdr_mac_80211). This constraint 
 * is verified by some special stuff in hdr-mac-80211.cc
 */

#include <stdint.h>

class Packet;

#define HDR_MAC_80211(p) ((hdr_mac_80211 *)hdr_mac::access(p))

enum mac_80211_packet_type {
	MAC_80211_CTL_RTS,
	MAC_80211_CTL_CTS,
	MAC_80211_CTL_ACK,
	MAC_80211_DATA,
	MAC_80211_MGT_BEACON,
	MAC_80211_MGT_ASSOCIATION_REQUEST,
	MAC_80211_MGT_ASSOCIATION_RESPONSE,
	MAC_80211_MGT_DISASSOCIATION,
	MAC_80211_MGT_REASSOCIATION_REQUEST,
	MAC_80211_MGT_REASSOCIATION_RESPONSE,
	MAC_80211_MGT_PROBE_REQUEST,
	MAC_80211_MGT_PROBE_RESPONSE,
	MAC_80211_MGT_AUTHENTICATION,
	MAC_80211_MGT_DEAUTHENTICATION
};

class hdr_mac_80211 {
 public:
	void initialize (void);

	int getTxMode (void) const;
	void setTxMode (int mode);

	/* return 0 for broadcast. */
	int getDestination (void) const;
	int getFinalDestination (void) const;
	int getSource (void) const;
	uint16_t getDataType (void) const;
	enum mac_80211_packet_type getType (void) const;
	double getDuration (void) const;
	int getSequence (void) const;
	bool isRetry (void) const;

	void setDestination (int destination);
	void setFinalDestination (int destination);
	void setSource (int source);
	void setDataType (uint16_t type);
	void setType (enum mac_80211_packet_type type);
	void setDuration (double duration);
	void setSequence (int sequence);
	void setRetry (void);

	char const *getTypeString (void) const;
	
 private:
	enum mac_80211_packet_type m_type;
	/* this is a hack to avoid growing 
	 * larger than the normal Mac header. 
	 */
	unsigned int m_dataType : 16;
	unsigned int m_duration : 16;
	unsigned int m_sequence : 12;
	unsigned int m_fragment : 4;
	unsigned int m_retry    : 1;
	unsigned int m_txMode   : 10;
	int m_destination;
	int m_finalDestination;
	int m_source;
};


void setSequence (Packet *packet, int sequence);
void increaseSize (Packet *packet, int increment);
void decreaseSize (Packet *packet, int decrement);
void setSize (Packet *packet, int size);
void setTxMode (Packet *packet, int mode);
void setDestination (Packet *packet, int destination);	
void setFinalDestination (Packet *packet, int destination);
void setDuration (Packet *packet, double duration);
void setSource (Packet *packet, int source);
void setType (Packet *packet, enum mac_80211_packet_type type);
void setRetry (Packet *packet);
void initialize (Packet *packet);


int getSize (Packet *packet);
double getDuration (Packet *packet);
bool isRetry (Packet *packet);
int getSequence (Packet *packet);
int getDestination (Packet *packet);
int getFinalDestination (Packet *packet);
int getSource (Packet *packet);
int getTxMode (Packet *packet);
enum mac_80211_packet_type getType (Packet *packet);
char const *getTypeString (Packet *packet);

#endif /* HDRMAC_80211 */
