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

#ifndef FREE_SPACE_PROPAGATION_H
#define FREE_SPACE_PROPAGATION_H

class NetInterface;
class Packet;
class NodePosition;

class FreeSpacePropagation {
public:
	FreeSpacePropagation ();

	void setInterface (NetInterface *interface);

	void setTransmissionPower (Packet *packet, double txPower);

	double getReceptionPower (Packet *packet);

	void setTxGain (double txGain);
	void setRxGain (double rxGain);
	void setSystemLoss (double systemLoss);
	void setFrequency (double frequency);
	
private:
	double squareDistance (NodePosition *a, NodePosition *b);
	double getLambda (void);

	NetInterface *m_interface;
	double m_txGain;
	double m_rxGain;
	double m_systemLoss;
	double m_frequency;

	static const double PI;
	static const double SPEED_OF_LIGHT;
	static const double SQ_MIN_DISTANCE;
};

#endif /* FREE_SPACE_PROPAGATION_H */
