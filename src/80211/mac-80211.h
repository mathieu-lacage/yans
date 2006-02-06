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

#ifndef MAC_80211_H
#define MAC_80211_H

#include "mac.h"

class MacContainer;
class Packet;
class Handler;

class Mac80211 : public Mac
{
public:
	Mac80211 ();
	virtual ~Mac80211 ();
	// XXXX 
	virtual void Mac80211::recv (Packet *p, Handler *h);
	/* inherited from Mac (mac/mac.h) */
	virtual int hdr_dst (char* hdr, int dst = -2);
	virtual int hdr_src (char* hdr, int src = -2);
	virtual int hdr_type(char *hdr, u_int16_t type = 0);

	virtual int command(int argc, const char*const* argv);
private:
	void startContainer (void);
	MacContainer *m_container;
};


#endif /* MAC_80211_H */
