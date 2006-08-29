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

#ifndef HCCA_TXOP_H
#define HCCA_TXOP_H

#include <stdint.h>
#include <map>

#include "hdr-mac-80211.h"
#include "mac-handler.tcc"

class MacQueue80211e;
class Packet;
class MacLowTransmissionListener;
class MacLow;
class MacStation;
class MacParameters;
class NetInterface80211;

class HccaTxop 
{
public:
	HccaTxop ();

	void setInterface (NetInterface80211 *interface);

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
	friend class MyQosNullTransmissionListener;

	MacLow *low (void);
	MacParameters *parameters (void);
	double now (void) const;
	MacStation *lookupDestStation (Packet *packet) const;
	bool enoughTimeFor (Packet *packet);
	bool enoughTimeFor (double duration);
	double getDurationIdLeft (void);
	void initializeTxop (double txopLimit, int tsid);
	bool isTxopFinished (void);
	void setCurrentTsid (uint8_t tsid);
	uint8_t getCurrentTsid (void);
	bool txCurrent (void);
	void dropCurrentPacket (void);
	Packet *getQosNullFor (int destination);
	void tryToSendQosNull (void);
	double calculateTxDuration (int txMode, uint32_t size);
	int getAckModeForDataMode (int destination, int txMode);


	void txopStartTimer (void);
	void gotCTS (double snr, int txMode);
	void missedCTS (void);
	void gotACK (double snr, int txMode);
	void missedACK (void);
	void gotQosNullAck (void);
	void missedQosNullAck (void);
	void startNext (void);

	NetInterface80211 *m_interface;
	uint8_t m_currentTsid;
	int m_SLRC;
	double m_txopLimit;
	double m_txopStartTime;
	Packet *m_currentTxPacket;
	StaticHandler<HccaTxop> *m_txopStart;
	MacLowTransmissionListener *m_transmissionListener;
	MacLowTransmissionListener *m_qosNullTransmissionListener;
	std::map<uint8_t, MacQueue80211e *, std::less<uint8_t> >m_tsQueues;
	std::map<enum ac_e, MacQueue80211e *, std::less<enum ac_e> >m_acQueues;
};

#endif /* HCCA_TXOP_H */
