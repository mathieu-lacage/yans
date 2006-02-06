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

#include "transmission-mode.h"
#include "base-transmission-mode.h"
#include "cck-mode.h"

#include <math.h>

CCKMMode::CCKMMode (double signalSpread)
	: BaseTransmissionMode (signalSpread, 5e6)
{}
CCKMMode::~CCKMMode ()
{}

double CCKMMode::chunkSuccessRate (double snr, unsigned int nbits)
{
	double pew = 7 * erfc (sqrt (4*snr)) + 0.5 * erfc (sqrt (8 * snr));
	double ber = 1 - pow (1 - pew, 0.25);
	double csr = pow (1-ber, nbits);
	return csr;
}

