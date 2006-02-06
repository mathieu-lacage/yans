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

#include "mac-high-station.h"

class MacContainer;
class Packet;
class HccaTxop;

class MacHighQStation : public MacHighStation {
public:
	MacHighQStation (MacContainer *container, int apAddress);
	virtual ~MacHighQStation ();

private:
	virtual void enqueueToLow (Packet *packet);
	virtual void gotCFPoll (Packet *packet);
	virtual void flush (void);

	bool isAcActive (uint8_t UP);
	bool isStreamActive (uint8_t TSID);

	HccaTxop *m_hcca;
};

#endif /* MAC_HIGH_QSTATION_H */
