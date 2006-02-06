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
#ifndef MAC_HIGH_QSTATION_H
#define MAC_HIGH_QSTATION_H

#include <map>

#include "mac-high-station.h"
#include "hdr-mac-80211.h"


class MacContainer;
class Packet;
class Dcf;
class MacDcfParameters;
class MacQueue80211e;
class HccaTxop;
class TSpec;
class TSpecRequest;

class MacHighQStation : public MacHighStation {
public:
	MacHighQStation (MacContainer *container, int apAddress);
	virtual ~MacHighQStation ();

private:
	virtual void addTsRequest (TSpecRequest *request);
	virtual void enqueueToLow (Packet *packet);
	virtual void gotCFPoll (Packet *packet);
	virtual void gotBeacon (Packet *packet);
	virtual void gotAssociated (Packet *packet);
	virtual void gotReAssociated (Packet *packet);
	virtual void gotAddTsResponse (Packet *packet);
	virtual void gotDelTsResponse (Packet *packet);
	virtual void flush (void);


	void updateEDCAParameters (unsigned char const *buffer);
	void createAC (enum ac_e ac);
	void createTS (TSpec *tspec);
	bool isTsActive (uint8_t TSID);
	void queueTS (uint8_t tsid, Packet *packet);
	void queueAC (enum ac_e ac, Packet *packet);

	MacQueue80211e *m_dcfQueues[4];
	MacDcfParameters *m_dcfParameters[4];
	Dcf *m_dcfs[4];
	HccaTxop *m_hcca;
	std::map<uint8_t, pair<TSpec *, MacQueue80211e *>, std::less<uint8_t> > m_ts;
};

#endif /* MAC_HIGH_QSTATION_H */
