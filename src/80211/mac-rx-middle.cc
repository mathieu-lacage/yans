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

#include "mac-rx-middle.h"
#include "mac-container.h"
#include "mac-low.h"
#include "mac-high.h"
#include "mac-stations.h"
#include "mac-station.h"

#define nopeMAC_MIDDLE_TRACE 1

#ifdef MAC_MIDDLE_TRACE
# define TRACE(format, ...) \
	printf ("MAC MIDDLE %d " format "\n", m_container->selfAddress (), ## __VA_ARGS__);
#else /* MAC_TRACE */
# define TRACE(format, ...)
#endif /* MAC_TRACE */


class MyMacLowReceptionListener : public MacLowReceptionListener
{
public:
	MyMacLowReceptionListener (MacRxMiddle *middle)
		: m_middle (middle) {}
	virtual ~MyMacLowReceptionListener ()
	{}

	virtual void gotPacket (int from, double snr, int txMode){
		m_middle->gotPacket (from, snr, txMode);
	}
	virtual void gotData (Packet *packet) {
		m_middle->gotData (packet);
	}
	virtual Packet *gotBlockAckReq (Packet *packet) {
#ifdef BACK
		return m_middle->gotBlockAckReq (packet);
#else /* BACK */
		return 0;
#endif /* BACK */
	}
private:
	MacRxMiddle *m_middle;
};

class OriginatorRxStatus {
public:
	OriginatorRxStatus () {
		/* this is a magic value necessary. */
		m_lastSequenceControl = 0xffff;
		m_deFragmenting = false;
	}
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

#ifdef BACK
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
	void fillBlockAckResp (Packet *packet, int sequenceControlStart) {
		dequeue<Packet *>::iterator i;
		int j;
		int n = 0;
		int prevSequenceNumber = sequenceControlStart >> 4;
		for (i = m_blocks.begin (), j = 0; i != m_blocks->end () && j < 64; i++) {
			int currentSequenceNumber = getSequenceControl (*i) >> 4;
			if (getSequenceControl (*i) >= sequenceControlStart) {
				n++;
				if (currentSequenceNumber > prevSequenceNumber) {
					j++;
					prevSequenceNumber = currentSequenceNumber;
				}
			}
		}
		packet->allocdata (n * sizeof (uint16_t));
		uint16_t *buffer = packet->accessdata ();
		for (i = m_blocks.begin (), j = 0; i != m_blocks->end () && j < n; i++) {
			if (getSequenceControl (*i) >= sequenceControlStart) {
				*buffer = getSequenceControl (*i);
				buffer++;
				j++;
			}
		}
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
	double m_inactivityTimer;
	int m_BABufferSize;
	bool m_blockAcking;
	dequeue<Packet *> m_blocks;
#endif /* BACK */
	bool m_deFragmenting;
	uint16_t m_lastSequenceControl;
	int m_rxSize;
};


MacRxMiddle::MacRxMiddle (MacContainer *container)
	: m_container (container)
{
	MyMacLowReceptionListener *listener = new MyMacLowReceptionListener (this);
	container->macLow ()->setReceptionListener (listener);
}

void
MacRxMiddle::forwardToHigh (Packet *packet)
{
	m_container->macHigh ()->receiveFromMacLow (packet);
}

void
MacRxMiddle::dropPacket (Packet *packet)
{
	Packet::free (packet);
}

bool
MacRxMiddle::sequenceControlSmaller (int seqca, int seqcb)
{
	int seqa = seqca >> 4;
	int seqb = seqcb >> 4;
	int delta = seqb - seqa;
	TRACE ("seqb: %d, seqa: %d, delta: %d", seqb, seqa, delta);
	if (delta <= 0 && delta < -2048) {
		return true;
	} else if (delta >= 0 && delta < 2048) {
		return true;
	} else {
		return false;
	}
}


OriginatorRxStatus *
MacRxMiddle::lookupQos (int source, int TID) 
{
	OriginatorRxStatus *originator;
	originator = m_qosOriginatorStatus[make_pair(source, TID)];
	if (originator == 0) {
		originator = new OriginatorRxStatus ();
		m_qosOriginatorStatus[make_pair(source, TID)] = originator;
	}
	return originator;
}

OriginatorRxStatus *
MacRxMiddle::lookup (int source) 
{
	OriginatorRxStatus *originator;
	originator = m_originatorStatus[source];
	if (originator == 0) {
		originator = new OriginatorRxStatus ();
		m_originatorStatus[source] = originator;
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

bool
MacRxMiddle::handleDuplicates (Packet *packet, OriginatorRxStatus *originator)
{
	if (originator->getLastSequenceControl () == getSequenceControl (packet)) {
		TRACE ("dump duplicate 0x%x", getSequenceControl (packet));
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
				TRACE ("accumulate fragment 0x%x %d", 
				       getSequenceControl (packet), getSize (packet));
				originator->accumulateFragment (getSize (packet));
				originator->setSequenceControl (getSequenceControl (packet));
				dropPacket (packet);
			} else {
				TRACE ("drop invalid fragment");
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
				TRACE ("accumulate last fragment 0x%x %d", 
				       getSequenceControl (packet), getSize (packet));
				int finalSize = originator->accumulateLastFragment (getSize (packet));
				setSize (packet, finalSize);
				originator->setSequenceControl (getSequenceControl (packet));
				// pass-through to give to mac high.
				return false;
			} else {
				TRACE ("drop packet");
				dropPacket (packet);
			}
		}
	} else {
		if (getMoreFragments (packet)) {
			TRACE ("accumulate first fragment 0x%x %d", 
			       getSequenceControl (packet), getSize (packet));
			originator->accumulateFirstFragment (getSize (packet));
			originator->setSequenceControl (getSequenceControl (packet));
			dropPacket (packet);
		} else {
			TRACE ("no defrag -- passthrough");
			// pass-through to give to mac high.
			return false;
		}
	}
	return true;
}

#ifdef BACK
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
Packet *
MacRxMiddle::gotBlockAckReq (Packet *packet)
{
	OriginatorRxStatus *originator = lookup (packet);
	Packet *resp;
	if (originator->isBlockAcking ()) {
		resp = Packet::alloc ();
		setType (resp, MAC_80211_CTL_BACKRESP);
		setSource (resp, getDestination (packet));
		setDestination (resp, getSource (packet));
		setTID (resp, getTID (packet));
		originator->fillBlockAckResp (resp, getBACKsequenceControl (packet));

		originator->indicateBlocks (resp);
		// and forward to mac content of rx buffer.
	} else {
		// this will allow the calling MacLow to
		// send an ACK instead.
		resp = 0;
		// queue delba
		
	}
	return resp;
}
#endif /* BACK */

void 
MacRxMiddle::gotPacket (int from, double snr, int txMode)
{
	m_container->stations ()->lookup (from)->reportRxOk (snr, txMode);
}

void 
MacRxMiddle::gotData (Packet *packet)
{
	OriginatorRxStatus *originator = lookup (packet);
	switch (getType (packet)) {
	case MAC_80211_MGT_ADDBA_REQUEST:
		TRACE ("got addba req");
		originator->setSequenceControl (getSequenceControl (packet));
		dropPacket (packet);
		break;
	case MAC_80211_DATA:
		assert (sequenceControlSmaller (originator->getLastSequenceControl (), getSequenceControl (packet)));
		// filter duplicates.
		if (!handleDuplicates (packet, originator) &&
		    //!handleBlockAck (packet, originator) &&
		    !handleFragments (packet, originator)) {
			TRACE ("forwarding data from %d", getSource (packet));
			originator->setSequenceControl (getSequenceControl (packet));
			forwardToHigh (packet);
		}
		break;
	default:
		TRACE ("forwarding %s", getTypeString (packet));
		originator->setSequenceControl (getSequenceControl (packet));
		forwardToHigh (packet);
		break;
	}

	return;
}
