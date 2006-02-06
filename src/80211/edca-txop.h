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

#ifndef EDCA_TXOP_H
#define EDCA_TXOP_H

#include <stdint.h>

class Dcf;
class MacQueue80211e;
class Packet;
class MacLowTransmissionListener;
class MacStation;
class MacContainer;
class MacLow;
class MacParameters;

class EdcaTxop 
{
public:
	EdcaTxop (Dcf *dcf, MacQueue80211e *queue, MacContainer *container);

private:
	friend class MyEdcaAccessListener;
	friend class MyEdcaTransmissionListener;

	MacLow *low (void);
	MacParameters *parameters (void);
	double now (void);
	double calculateTxDuration (uint32_t size, int txMode);
	double calculateCurrentTxDuration (void);
	double calculateNextTxDuration (Packet *packet);
	double enoughCurrentTime (void);
	double enoughTime (double txDuration);
	void tryToSendOnePacket (void);

	/* event handlers */
	void accessGrantedNow (void);
	bool accessingAndWillNotify (void);
	bool accessNeeded (void);
	void gotCTS (double snr, int txMode);
	void missedCTS (void);
	void gotACK (double snr, int txMode);
	void missedACK (void);
	void startNext (void);
	void cancel (void);

	bool needRTS (void);
	void dropCurrentPacket (void);
	MacStation *lookupDestStation (Packet *packet);

	Dcf *m_dcf;
	MacQueue80211e *m_queue;
	MacContainer *m_container;
	Packet *m_currentTxPacket;
	MacLowTransmissionListener *m_transmissionListener;
	int m_SSRC;
	int m_SLRC;
	bool m_firstPacketInBurst;
	bool m_lastPacketInBurst;
	double m_startTxopTime;
};



#endif /* EDCA_TXOP_H */
