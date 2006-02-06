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
#ifndef MAC_HIGH_QSTA_H
#define MAC_HIGH_QSTA_H

#include <map>

#include "mac-high-sta.h"
#include "hdr-mac-80211.h"


class Packet;
class Dcf;
class MacDcfParameters;
class MacQueue80211e;
class HccaTxop;
class TSpec;
class TSpecRequest;

class MacHighQsta : public MacHighSta {
public:
	MacHighQsta (int apAddress);
	virtual ~MacHighQsta ();

	void setInterface (NetInterface80211 *interface);

	virtual void addTsRequest (TSpecRequest *request);
	virtual void delTsRequest (TSpecRequest *request);

private:
	virtual void enqueueToLow (Packet *packet);
	virtual void gotCFPoll (Packet *packet);
	virtual void gotBeacon (Packet *packet);
	virtual void gotAssociated (Packet *packet);
	virtual void gotReAssociated (Packet *packet);
	virtual void gotAddTsResponse (Packet *packet);
	virtual void gotDelTsResponse (Packet *packet);
	virtual void flush (void);


	void updateEDCAParameters (unsigned char const *buffer);
	void createAC (enum ac_e ac, NetInterface80211 *interface);
	void createTS (TSpec *tspec);
	bool isTsActive (uint8_t TSID);
	void queueTS (uint8_t tsid, Packet *packet);
	void queueAC (enum ac_e ac, Packet *packet);

	NetInterface80211 *m_interface;
	MacQueue80211e *m_dcfQueues[4];
	MacDcfParameters *m_dcfParameters[4];
	Dcf *m_dcfs[4];
	HccaTxop *m_hcca;
	std::map<uint8_t, pair<TSpec *, MacQueue80211e *>, std::less<uint8_t> > m_ts;
};

#endif /* MAC_HIGH_QSTA_H */
