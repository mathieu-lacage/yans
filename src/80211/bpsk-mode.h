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
 * Author: Mathieu Lacage, <mathieu.lacage@sophia.inria.fr>
 */

#ifndef _ns_bpsk_mode_h_
#define _ns_bpsk_mode_h_

#include "transmission-mode.h"
#include "base-transmission-mode.h"

class BPSKMode : public BaseTransmissionMode
{
public:
	BPSKMode (double signalSpread, double rate);
	virtual ~BPSKMode ();
	virtual double chunkSuccessRate (double snr, unsigned int nbits);
private:

};

class BPSKFECMode : public FECBaseTransmissionMode
{
public:
	BPSKFECMode (unsigned int dFree, unsigned int adFree, 
		     double signalSpread, double rate, double codingRate);
	virtual ~BPSKFECMode ();
	virtual double chunkSuccessRate (double snr, unsigned int nbits);
private:
	// XXXX
	unsigned int dFree_;
	unsigned int adFree_;
};


#endif // _ns_bpsk_mode_h_
