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

#ifndef CABLE_H
#define CABLE_H

#include <list>
#include "ref-count.tcc"

namespace yans {

class Packet;
class EthernetNetworkInterface;

class Cable {
public:
	/* The default is:
	 * 100m+Mb/s
	 */
	Cable ();
	/* 
	 * @length: meters
	 * @bandwidth: bits/second
	 */
	Cable (double length, double bandwidth);

	void ref (void);
	void unref (void);

	void connect_to (EthernetNetworkInterface *a,
			 EthernetNetworkInterface *b);

	void send (Packet *packet, EthernetNetworkInterface *sender);

private:
	void recv (Packet *packet, EthernetNetworkInterface *sender);
	static const double SPEED_OF_LIGHT;
	EthernetNetworkInterface *m_a;
	EthernetNetworkInterface *m_b;
	RefCount<Cable> m_ref;
	double m_length;
	double m_bandwidth;
};

}; // namespace yans


#endif /* CABLE_H */
