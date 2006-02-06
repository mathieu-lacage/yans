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
	virtual double getRate (void) const {return rate_;}

private:
	double signalSpread_;
	double rate_;
protected:
	double BPSKBER (double SNR);
	double QAMBER (double SNR, unsigned int M);
	static double log2 (double v);
};


class FECBaseTransmissionMode : public BaseTransmissionMode
{
public:
	FECBaseTransmissionMode (double signalSpread, double rate);
	virtual ~FECBaseTransmissionMode ();
protected:
	double calculatePdOdd (double ber, unsigned int d);
	double calculatePdEven (double ber, unsigned int d);
	double calculatePd (double ber, unsigned int d);
};


#endif //_ns_base_transmission_mode_h_
