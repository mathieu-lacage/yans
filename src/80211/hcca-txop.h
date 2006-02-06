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

#ifndef HCCA_TXOP_H
#define HCCA_TXOP_H

#include <stdint.h>
#include <map>

#include "hdr-mac-80211.h"

class MacContainer;
class MacQueue80211e;
class Packet;
class MacLowTransmissionListener;
class MacLow;
class MacStation;

class HccaTxop 
{
public:
	HccaTxop (MacContainer *container);

	void tsAccessGranted (uint8_t TSID, double txopLimit);
	void addStream (MacQueue80211e *queue, uint8_t TSID);
	void deleteStream (uint8_t TSID);

	// XXX what is this method ? 
	// remove it ?
	void acAccessGranted (enum ac_e ac, double txopLimit);
	void addAccessCategory (MacQueue80211e *queue, enum ac_e accessCategory);
	void deleteAccessCategory (enum ac_e accessCategory);
private:
	friend class MyTransmissionListener;

	MacLow *low (void) const;
	double now (void) const;
	MacStation *lookupDestStation (Packet *packet) const;
	bool enoughTimeFor (Packet *packet);
	bool enoughTimeFor (double duration);
	double getDurationIdLeft (void);
	void startTxop (double txopLimit);
	bool isTxopFinished (void);
	void setCurrentTsid (uint8_t tsid);
	uint8_t getCurrentTsid (void);
	bool txCurrent (void);
	void dropCurrentPacket (void);

	void gotCTS (double snr, int txMode);
	void missedCTS (void);
	void gotACK (double snr, int txMode);
	void missedACK (void);
	void startNext (void);

	MacContainer *m_container;
	uint8_t m_currentTsid;
	int m_SLRC;
	double m_txopLimit;
	double m_txopStart;
	Packet *m_currentTxPacket;
	MacLowTransmissionListener *m_transmissionListener;
	std::map<uint8_t, MacQueue80211e *, std::less<uint8_t> >m_tsQueues;
	std::map<enum ac_e, MacQueue80211e *, std::less<enum ac_e> >m_acQueues;
};

#endif /* HCCA_TXOP_H */
