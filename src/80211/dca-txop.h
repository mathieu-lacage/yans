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
class MacLowTransmissionListener;
class MacStation;
class NetInterface80211;
class MacLow;
class MacParameters;

class DcaTxop 
{
public:
	DcaTxop (Dcf *dcf, MacQueue80211e *queue);

	void setInterface (NetInterface80211 *interface);

private:
	friend class MyDcaAccessListener;
	friend class MyDcaTransmissionListener;

	double now (void);

	MacLow *low (void);
	MacParameters *parameters (void);

	/* event handlers */
	void accessGrantedNow (void);
	bool accessingAndWillNotify (void);
	bool accessNeeded (void);
	void gotCTS (double snr, int txMode);
	void missedCTS (void);
	void gotACK (double snr, int txMode);
	void missedACK (void);
	void startNext (void);

	bool needRTS (void);
	bool needFragmentation (void);
	int getNFragments (void);
	int getLastFragmentSize (void);
	int getNextFragmentSize (void);
	int getFragmentSize (void);
	bool isLastFragment (void);
	void nextFragment (void);
	Packet *getFragmentPacket (void);
	void dropCurrentPacket (void);
	MacStation *lookupDestStation (Packet *packet);

	Dcf *m_dcf;
	MacQueue80211e *m_queue;
	NetInterface80211 *m_interface;
	Packet *m_currentTxPacket;
	MacLowTransmissionListener *m_transmissionListener;
	int m_SSRC;
	int m_SLRC;
	int m_fragmentNumber;
};



#endif /* DCA_TXOP_H */
