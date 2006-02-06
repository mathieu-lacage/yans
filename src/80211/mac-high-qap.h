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
#ifndef MAC_HIGH_QAP
#define MAC_HIGH_QAP

#include <map>

#include "mac-high-ap.h"
#include "mac-handler.tcc"
#include "hdr-mac-80211.h"

class MacContainer;
class Packet;
class MacDcfParameters;
class MacQueue80211e;
class Dcf;
class MacParameters;
class HccaTxop;
class QapScheduler;
class TSpec;

class DcfParameters {
public:
	uint8_t AIFSN;
	bool ACM;
	uint8_t ECWmin;
	uint8_t ECWmax;
	uint16_t txop;
};

class MacHighQap : public MacHighAp {
public:
	MacHighQap (MacContainer *container);
	virtual ~MacHighQap ();

	virtual void addTsRequest (TSpecRequest *request);
	virtual void delTsRequest (TSpecRequest *request);

	/* to be used by the qap-scheduler. */
	void addEDCAparameters (Packet *packet);

 private:
	MacParameters *parameters (void);
	Packet *getPacketFor (int destination);
	void createAC (enum ac_e ac);
	void createTS (TSpec *tspec);
	void destroyTS (uint8_t tsid);

	void queueAC (enum ac_e ac, Packet *packet);
	void queueTS (uint8_t tsid, Packet *packet);
	bool isTsActive (uint8_t tsid);
	void queueTsResponse (int destination, TSpecRequest *request,
			      enum mac_80211_packet_type type, 
			      enum mac_80211_request_status status);
	void queueAddTsResponseOk (int destination, 
				   TSpecRequest *request);
	void queueAddTsResponseRefused (int destination, 
					TSpecRequest *request);
	void queueDelTsResponseOk (int destination, 
				   TSpecRequest *request);
	void queueDelTsResponseRefused (int destination, 
					TSpecRequest *request);

	virtual void enqueueToLow (Packet *packet);
	virtual void forwardQueueToLow (Packet *packet);
	virtual void sendAssociationResponseOk (int destination);
	virtual void sendReAssociationResponseOk (int destination);
	virtual void sendProbeResponse (int destination);
	virtual void gotAddTsRequest (Packet *packet);
	virtual void gotDelTsRequest (Packet *packet);
	virtual void gotCFPoll (Packet *packet);

	MacDcfParameters *m_dcfParameters;
	MacQueue80211e *m_dcfQueues[4];
	Dcf *m_dcfs[4];
	DcfParameters m_staDcfParameters[4];
	HccaTxop *m_hcca;
	QapScheduler *m_scheduler;
	std::map<uint8_t, pair<TSpec *, MacQueue80211e *>, std::less<uint8_t> > m_ts;
};

#endif /* MAC_HIGH_QAP */
