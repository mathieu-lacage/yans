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

#ifndef "mac-rx-middle.h"

class MyReceptionListener
{
public:
	ReceptionListener (MacRxMiddle *middle)
		: m_middle (middle) {}
	virtual ~ReceptionListener ()
	{}

	virtual void gotPacket (double snr, int txMode){
		m_middle->gotPacket (snr, txMode);
	}
	virtual void gotData (Packet *packet) {
		m_middle->gotData (packet);
	}
	virtual Packet *gotBlockAckReq (Packet *packet) {
		return m_middle->gotBlockAckReq (packet);
	}
private:
	MacRxMiddle *m_middle;
};

class OriginatorRxStatus {
public:
	bool isDeFragmenting (void) {
		return m_deFragmenting;
	}
	void accumulateFirstFragment (int firstFragmentSize) {
		assert (!m_deFragmenting);
		m_deFragmenting = true;
		m_rxSize = firstFragmentSize;
	}
	int accumulateLastFragment (int fragmentSize) {
		m_deFragmenting = false;
		m_rxSize += fragmentSize;
		return m_rxSize;
	}
	void accumulateFragment (int fragmentSize) {
		m_rxSize += fragmentSize;
	}
	bool isNextFragment (uint16_t sequenceControl) {
		if ((sequenceControl >> 4) == (m_lastSequenceControl >> 4) &&
		    (sequenceControl & 0x0f) == ((m_lastSequenceControl & 0x0f)+1)) {
			return true;
		} else {
			return false;
		}
	}
	uint16_t getLastSequenceControl (void) {
		return m_lastSequenceControl;
	}
	void setSequenceControl (uint16_t sequenceControl) {
		m_lastSequenceControl = sequenceControl;
	}


	void setBABufferSize (int n) {
		m_BABufferSize = n;
	}
	void setInactivityTimer (double delay) {
		m_inactivityTimer = delay;
	}
	void resetInactivityTimer (double now) {
		m_lastInactivityTimerStart = now;
	}
	double isInactivityTimerExpired (double now) {
		double delay = now - m_lastInactivityTimerStart;
		if (m_inactivityTimer < delay) {
			return true;
		} else {
			return false;
		}
	}
	bool isBlockAcking (void) {
		return m_blockAcking;
	}
	bool accumulateFirstBlock (Packet *packet) {
		m_blockAcking = true;
		return appendBlock (packet);
	}
	bool accumulateLastBlock (Packet *packet) {
		m_blockAcking = false;
		return appendBlock (packet);
	}
	bool accumulateBlock (Packet *packet) {
		return appendBlock (packet);
	}
	Packet *getOldestCompletePacket (void) {
		/* Here, we have to defragment the first packet
		 * we can from the BA buffer.
		 */
		dequeue<Packet *>::iterator i;
		Packet *defragmented;
		int totalSize = 0;
		for (i = m_blocks.begin (); i != m_blocks->end (); i++) {
			totalSize += getSize (*i);
			if (!getMoreFragments (*i)) {
				defragmented = *i;
				setSize (defragmented, totalSize);
				m_blocks->erase (m_blocks->begin (), i);
				return defragmented;
				break;
			}
			Packet::free (*i);
		}
		m_blocks->erase (m_blocks->begin (), m_blocks->end ());
		return 0;
	}
private:
	bool appendBlock (Packet *packet) {
		m_blocks->push_back (packet);
		if (m_blocks->size () == m_BABufferSize) {
			return true;
		} else {
			return false;
		}
	}
	bool m_deFragmenting;
	uint16_t m_lastSequenceControl;
	int m_rxSize;
	double m_inactivityTimer;
	int m_BABufferSize;
	bool m_blockAcking;
	dequeue<Packet *> m_blocks;
};


MacRxMiddle::MacRxMiddle (MacLow *low, MacHigh *high,
			  MacStations *stations)
	: m_low (low),
	  m_high (high),
	  m_stations (stations)
{
	listener = new MyReceptionListener (this);
	m_low->setReceptionListener (listener);
}

OriginatorRxStatus *
MacRxMiddle::lookupQoS (int source, int TID) 
{
	OriginatorRxStatus *originator;
	originator = m_qosOriginatorStatus[make_pair(source, TID)];
	if (originator == 0) {
		originator = m_qosOriginatorStatus[make_pair(source, TID)] = new OriginatorRxStatus ();
	}
	return originator;
}

OriginatorRxStatus *
MacRxMiddle::lookup (int source) 
{
	OriginatorRxStatus *originator;
	originator = m_originatorStatus[source];
	if (originator == 0) {
		originator = m_originatorStatus[source] = new OriginatorRxStatus ();
	}
	return originator;
}

OriginatorRxStatus *
MacRxMiddle::lookup (Packet *packet)
{
	OriginatorRxStatus *originator;
	if (isQos (packet)) {
		originator = lookupQos (getSource (packet), getTID (packet));
	} else {
		originator = lookup (getSource (packet));
	}
	return originator;
}

/* Return true is packet has been handled by BlockAck
 * machinery. Return false otherwise.
 */
bool
MacRxMiddle::handleBlockAck (Packet *packet, OriginatorRxStatus *originator)
{
	if (originator->isBlockAcking ()) {
		if (isBlockAck (packet)) {
			if (originator->isInactivityTimerExpired (now ())) {
				// XXX cancel BA and send delba.
				dropPacket (packet);
			} else {
				if (originator->accumulateBlock (packet)) {
					Packet *old = originator->getOldestCompletePacket ();
					if (old != 0) {
						// XXX forward to MAC directly.
					}
				}
				originator->resetInactivityTimer (now ());
			}
		} else {
			if (originator->isInactivityTimerExpired (now ())) {
				// XXX cancel BA and send delba.
			}
			/* accumulate in block ack anyway but do not
			   update the inactivity timer. 
			*/
			if (originator->accumulateBlock (packet)) {
				Packet *old = originator->getOldestCompletePacket ();
				if (old != 0) {
					// XXX forward to MAC directly.
				}
			}
		}
	} else {
		if (isBlockAck (packet)) {
			// XXX cancel BA and send delba.
			dropPacket (packet);
		} else {
			return false;
		}
	}
	return true;
}

bool
MacRxMiddle::handleDuplicates (Packet *packet, OriginatorRxStatus *originator)
{
	if (originator->getLastSequenceControl () == getSequenceControl (packet)) {
		dropPacket (packet);
		return true;
	}
	return false;
}

bool
MacRxMiddle::handleFragments (Packet *packet, OriginatorRxStatus *originator)
{
	// defragment
	if (originator->isDeFragmenting ()) {
		if (getMoreFragments (packet)) {
			if (originator->isNextFragment (getSequenceControl (packet))) {
				originator->accumulateFragment (getSize (packet));
				originator->setSequenceControl (getSequenceControl (packet));
				dropPacket (packet);
			} else {
				dropPacket (packet);
			}
		} else {
			if (originator->isNextFragment (getSequenceControl (packet))) {
				/* Each fragment is a copy of the original non-fragmented
				 * packet except for its size which is set to the size of
				 * the real fragment. This allows us to re-use the last 
				 * fragment to change its size back to the original packet 
				 * size and to pass it up to the higher-level layers.
				 */
				int finalSize = originator->accumulateLastFragment (getSize (packet));
				setSize (packet, finalSize);
				originator->setSequenceControl (getSequenceControl (packet));
				// pass-through to give to mac high.
				return false;
			} else {
				dropPacket (packet);
			}
		}
	} else {
		if (getMoreFragments (packet)) {
			originator->accumulateFirstFragment (getSize (packet));
			originator->setSequenceControl (getSequenceControl (packet));
			dropPacket (packet);
		} else {
			// pass-through to give to mac high.
			return false;
		}
	}
	return true;
}

Packet *
MacRxMiddle::gotBlockAckReq (Packet *packet)
{
	OriginatorRxStatus *originator = lookup (packet);
	Packet *resp;
	if (originator->isBlockAcking ()) {
		resp = allocBlockAckResp (originator);
		// send BlockAckResp
		// and forward to mac content of rx buffer.
	} else {
		resp = 0;
		// should set resp to ack.
		// queue delba
		
	}
	return resp;
}

void 
MacRxMiddle::gotData (Packet *packet)
{
	OriginatorRxStatus *originator = lookup (packet);
	switch (getType (packet)) {
	case MAC_80211_DATA:
		assert (station->getLastSequenceControl () <= getSequenceControl (packet));
		// filter duplicates.
		if (!handleDuplicates (packet, originator) &&
		    !handleBlockAck (packet, originator) &&
		    !handleFragments (packet, originator)) {
			// XXX forward to MAC high.
		}
		break;
	default:
		// XXX forward to MAC high.
		break;
	}

	return;
 drop:
	dropPacket (packet);
}
