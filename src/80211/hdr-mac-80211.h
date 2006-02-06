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
	MAC_80211_CTL_RTS = 0,
	MAC_80211_CTL_CTS,
	MAC_80211_CTL_ACK,
	MAC_80211_CTL_BACKREQ,
	MAC_80211_CTL_BACKRESP,
	MAC_80211_DATA,
	MAC_80211_MGT_CFPOLL,
	MAC_80211_MGT_BEACON,
	MAC_80211_MGT_ASSOCIATION_REQUEST,
	MAC_80211_MGT_ASSOCIATION_RESPONSE,
	MAC_80211_MGT_DISASSOCIATION,
	MAC_80211_MGT_REASSOCIATION_REQUEST,
	MAC_80211_MGT_REASSOCIATION_RESPONSE,
	MAC_80211_MGT_PROBE_REQUEST,
	MAC_80211_MGT_PROBE_RESPONSE,
	MAC_80211_MGT_AUTHENTICATION,
	MAC_80211_MGT_DEAUTHENTICATION,
	MAC_80211_MGT_ADDTS_REQUEST,
	MAC_80211_MGT_ADDTS_RESPONSE,
	MAC_80211_MGT_DELTS_REQUEST,
	MAC_80211_MGT_DELTS_RESPONSE,
	MAC_80211_MGT_ADDBA_REQUEST,
	MAC_80211_MGT_ADDBA_RESPONSE,
	MAC_80211_MGT_DELBA_REQUEST,
	MAC_80211_MGT_DELBA_RESPONSE
};

enum mac_80211_request_status {
	MAC_80211_ADDTS_OK,
	MAC_80211_ADDTS_FAILED,
	MAC_80211_DELTS_OK,
	MAC_80211_DELTS_FAILED,
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
	bool getMoreFragments (void) const;
	bool isRetry (void) const;
	int getSequenceControl (void) const;
	uint8_t getTID (void) const;
	double getTxopLimit (void) const;
	bool isBlockAck (void) const;
	bool isNoAck (void) const;
	bool isNormalAck (void) const;
	bool isQos (void) const;

	void setDestination (int destination);
	void setFinalDestination (int destination);
	void setSource (int source);
	void setDataType (uint16_t type);
	void setType (enum mac_80211_packet_type type);
	void setDuration (double duration);
	void setSequenceNumber (int sequence);
	void setFragmentNumber (int fragmentNumber);
	void setMoreFragments (bool);
	void setRetry (void);
	void setTID (uint8_t tid);
	void setTxopLimit (double);
	void setNoAck (void);
	void setBlockAck (void);
	void setNormalAck (void);

	char const *getTypeString (void) const;
	
 private:
	enum {
		ACK_NORMAL,
		ACK_NO,
		ACK_BLOCK
	};
	enum mac_80211_packet_type m_type;
	/* this is a hack to avoid growing 
	 * larger than the normal Mac header. 
	 */
	unsigned int m_dataType        : 16;
	unsigned int m_duration        : 16;
	unsigned int m_sequenceControl : 16;
	unsigned int m_retry           : 1;
	unsigned int m_moreFragments   : 1;
	unsigned int m_txMode          : 10;
	unsigned int m_tid             : 4;
	unsigned int m_ackMode         : 2;
	unsigned int m_qos             : 1;
	unsigned int m_txopLimit       : 8;
	int m_destination;
	int m_finalDestination;
	int m_source;
};


void setTID (Packet *packet, uint8_t tid);
void setTxopLimit (Packet *packet, double txopLimit);
void setNoAck (Packet *packet);
void setBlockAck (Packet *packet);
void setNormalAck (Packet *packet);
void setSequenceNumber (Packet *packet, int sequence);
void setFragmentNumber (Packet *packet, int fragment);
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
void setMoreFragments (Packet *packet, bool moreFragments);
void initialize (Packet *packet);


int getSize (Packet *packet);
double getDuration (Packet *packet);
bool isRetry (Packet *packet);
int getSequenceControl (Packet *packet);
int getDestination (Packet *packet);
int getFinalDestination (Packet *packet);
int getSource (Packet *packet);
int getTxMode (Packet *packet);
enum mac_80211_packet_type getType (Packet *packet);
bool getMoreFragments (Packet *packet);
uint8_t getTID (Packet *packet);
double getTxopLimit (Packet *packet);
bool isBlockAck (Packet *packet);
bool isNoAck (Packet *packet);
bool isNormalAck (Packet *packet);
bool isQos (Packet *packet);

uint8_t getRequestedTID (Packet *packet);

bool isData (Packet *packet);
bool isManagement (Packet *packet);
bool isControl (Packet *packet);

bool isData (enum mac_80211_packet_type type);
bool isManagement (enum mac_80211_packet_type type);
bool isControl (enum mac_80211_packet_type type);

enum ac_e {
	AC_BE = 0,
	AC_BK = 1,
	AC_VI = 2,
	AC_VO = 3,
	/* This special AC is reserved to allow the QAP to send
	 * frames without queuing them in any of the AC or TS 
	 * queues. It is speficically used to send all the 
	 * management frames which require precise timing such as:
	 *  - BEACON
	 *  - CF-Poll
	 * The reason why we do not re-use any of the other AC_*
	 * is because the frames we want to send cannot suffer from
	 * being pushed in a queue (they need to be sent immediately)
	 * and they _must_ all share a single sequence counter at
	 * the receiver side (since, per section 9.2.9 of the 802.11e
	 * standard, duplicate detection is done on a per-TID basis).
	 *
	 * This explanation is not very clear but the bottomline is 
	 * that, if you try to bypass the AC and TS queues, you need
	 * to maintain your own monotonically increasing sequence
	 * counter and you need to assign it to a specific TID. Thus
	 * the idea of re-using an otherwise unused TID which belongs
	 * to the Voice category. If you don't do that,
	 * the receiver's duplicate detection algorithm will burst
	 * in flames and randomly drop non-duplicate packets.
	 *
	 * Another solution might have been to extract the sequence
	 * counter from one of the existing queues, re-use its TID
	 * for our special frames and increase its sequence counter
	 * accordingly. This would probably be rather tricky to 
	 * implement and seems much less clean that using this magic
	 * TID.
	 *
	 * Big thanks to the unknown member of the 802.11e commitee 
	 * who decided to keep some spare Access Categories.
	 */
	AC_SPECIAL = 4
};
char const *getTypeString (Packet *packet);
char const *getAcString (Packet *packet);
void setAC (Packet *packet, enum ac_e ac);
enum ac_e getAC (Packet *packet);

#endif /* HDRMAC_80211 */
