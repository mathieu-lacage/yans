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

#ifndef DCA_TXOP_H
#define DCA_TXOP_H

#include <stdint.h>

class Dcf;
class MacQueue80211e;
class Packet;
class MacLow;
class MacParameters;
class TransmissionListener;
class MacStation;
class MacHigh;

class DcaTxop 
{
public:
	DcaTxop (Dcf *dcf, MacQueue80211e *queue, MacLow *low, MacParameters *parameters);

private:
	friend class MyAccessListener;
	friend class MyTransmissionListener;

	/* event handlers */
	void accessGrantedNow (void);
	void gotCTS (double snr, int txMode);
	void missedCTS (void);
	void gotACK (double snr, int txMode);
	void missedACK (void);
	void txCompletedAndSIFS (void);
	void gotBlockAckStart (double snr);
	void gotBlockAck (int sequence);
	void gotBlockAckEnd (void);
	void missedBlockAck (void);

	bool needRTS (void);
	bool needFragmentation (void);
	int getNFragments (void);
	int getLastFragmentSize (void);
	int getFragmentSize (void);
	bool isLastFragment (void);
	void nextFragment (void);
	Packet *getFragmentPacket (void);
	void dropCurrentPacket (void);
	MacStation *lookupDestStation (Packet *packet);

	Dcf *m_dcf;
	MacQueue80211e *m_queue;
	MacLow *m_low;
	MacHigh *m_high;
	Packet *m_currentTxPacket;
	MacParameters *m_parameters;
	TransmissionListener *m_transmissionListener;
	int m_SSRC;
	int m_SLRC;
	int m_fragmentNumber;
	uint16_t m_sequence;
};



#endif /* DCA_TXOP_H */
