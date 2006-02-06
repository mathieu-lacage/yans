/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*-  *
 *
 * Copyright (c) 2004 INRIA
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

#ifndef  _ns_base_transmission_mode_h_
#define  _ns_base_transmission_mode_h_

#include "transmission-mode.h"

/* Code which can be shared by a lot of transmission modes goes into
 * these classes.
 */

class BaseTransmissionMode : public TransmissionMode
{
public:
	BaseTransmissionMode (double signalSpread, double rate);
	virtual ~BaseTransmissionMode () {}
	virtual double getSignalSpread (void) const {return signalSpread_;}
	virtual double getDataRate (void) const {return rate_;}

private:
	double signalSpread_;
	double rate_;
protected:
	double getRate (void) const {return rate_;}
	double BPSKBER (double SNR);
	double QAMBER (double SNR, unsigned int M);
	static double log2 (double v);
};


class FECBaseTransmissionMode : public BaseTransmissionMode
{
public:
	FECBaseTransmissionMode (double signalSpread, double rate, double codingRate);
	virtual ~FECBaseTransmissionMode ();
	virtual double getDataRate (void) const {return getRate () * m_codingRate;}
protected:
	double calculatePdOdd (double ber, unsigned int d);
	double calculatePdEven (double ber, unsigned int d);
	double calculatePd (double ber, unsigned int d);

private:
	double m_codingRate;
};


#endif //_ns_base_transmission_mode_h_
