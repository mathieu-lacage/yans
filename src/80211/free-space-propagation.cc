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

#include "free-space-propagation.h"
#include "net-node.h"
#include "hdr-mac-80211.h"
#include "net-interface.h"

const double FreeSpacePropagation::PI = 3.1415;
const double FreeSpacePropagation::SPEED_OF_LIGHT = 3e8; // m/s
const double FreeSpacePropagation::SQ_MIN_DISTANCE = 1e-4;



FreeSpacePropagation::FreeSpacePropagation ()
{}

void 
FreeSpacePropagation::setInterface (NetInterface *interface)
{
	m_interface = interface;
}

double
FreeSpacePropagation::squareDistance (NodePosition *a, NodePosition *b)
{
	double dx = b->getX () - a->getX ();
	double dy = b->getY () - a->getY ();
	double dz = b->getZ () - a->getZ ();
	double squareD = dx*dx + dy*dy + dz*dz;
	return squareD;
}

void 
FreeSpacePropagation::setTxGain (double txGain)
{
	m_txGain = txGain;
}
void 
FreeSpacePropagation::setRxGain (double rxGain)
{
	m_rxGain = rxGain;
}
void 
FreeSpacePropagation::setSystemLoss (double systemLoss)
{
	m_systemLoss = systemLoss;
}
void 
FreeSpacePropagation::setFrequency (double frequency)
{
	m_frequency = frequency;
}
double
FreeSpacePropagation::getLambda (void)
{
	double lambda = SPEED_OF_LIGHT / m_frequency;
	return lambda;
}
double
FreeSpacePropagation::getReceptionPower (Packet *packet)
{
	double txAirPower = HDR_MAC_80211 (packet)->getTxAirPower ();
	NodePosition txPosition, rxPosition;
	HDR_MAC_80211 (packet)->peekTxNodePosition (&txPosition);
	m_interface->peekPosition (&rxPosition);
	double sqDistance = squareDistance (&txPosition, &rxPosition);

	if (sqDistance <= SQ_MIN_DISTANCE) {
		return txAirPower * m_rxGain / m_systemLoss;
	}
        /*
         * Friis free space equation:
         *
         *       Pt * Gt * Gr * (lambda^2)
         *   P = --------------------------
         *       (4 * pi * d)^2 * L
         */
	double lambda = getLambda ();
	double numerator = txAirPower * m_rxGain * lambda *lambda;
	double denominator = 16 * PI * PI * sqDistance * m_systemLoss;
	return numerator / denominator;
}

void
FreeSpacePropagation::setTransmissionPower (Packet *packet, double txPower)
{
	double txAirPower = txPower * m_txGain;
	HDR_MAC_80211 (packet)->setTxAirPower (txAirPower);
	NodePosition txPosition;
	m_interface->peekPosition (&txPosition);
	HDR_MAC_80211 (packet)->setTxNodePosition (&txPosition);
}
