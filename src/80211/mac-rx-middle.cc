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

#include "mac-rx-middle.h"
#include "mac-low.h"
#include "mac-high.h"
#include "mac-station.h"
#include "mac-traces.h"
#include "net-interface-80211.h"
#include "common.h"

#include "packet.h"

#ifndef MAC_MIDDLE_TRACE
#define nopeMAC_MIDDLE_TRACE 1
#endif /* MAC_MIDDLE_TRACE */

#ifdef MAC_MIDDLE_TRACE
# define TRACE(format, ...) \
	printf ("MAC MIDDLE %d " format "\n", m_interface->getMacAddress (), ## __VA_ARGS__);
#else /* MAC_TRACE */
# define TRACE(format, ...)
#endif /* MAC_TRACE */


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

private:
	bool m_deFragmenting;
	uint16_t m_lastSequenceControl;
	int m_rxSize;
};


MacRxMiddle::MacRxMiddle ()
{}

void 
MacRxMiddle::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;
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
MacRxMiddle::lookupNqos (int source) 
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
	if (isQos (packet) &&
            isData (packet) &&
            getDestination (packet) != MAC_BROADCAST) {
                /* only for qos data broadcast frames */
		originator = lookupQos (getSource (packet), getTID (packet));
	} else {
                /* - management frames
                 * - qos data broadcast frames
                 * - nqos data frames
                 * see section 7.1.3.4.1
                 */
		originator = lookupNqos (getSource (packet));
	}
	return originator;
}

bool
MacRxMiddle::handleDuplicates (Packet *packet, OriginatorRxStatus *originator)
{
	if (originator->getLastSequenceControl () == getSequenceControl (packet)) {
		TRACE ("dump duplicate seq=0x%x tid=%u", 
		       getSequenceControl (packet),
		       getTID (packet));
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

void
MacRxMiddle::sendUp (Packet *packet)
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
			TRACE ("forwarding data from %d seq=0x%x tid=%u", 
			       getSource (packet), getSequenceControl (packet),
			       getTID (packet));
			originator->setSequenceControl (getSequenceControl (packet));
			m_interface->high ()->receiveFromMacLow (packet);
		} else {
			dropPacket (packet);
		}
		break;
	default:
		TRACE ("forwarding %s seq=0x%x tid=%u",
		       getTypeString (packet), 
		       getSequenceControl (packet),
		       getTID (packet));
		originator->setSequenceControl (getSequenceControl (packet));
		m_interface->high ()->receiveFromMacLow (packet);
		break;
	}
}
