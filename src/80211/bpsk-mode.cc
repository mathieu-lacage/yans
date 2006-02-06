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

#include "bpsk-mode.h"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>


BPSKMode::BPSKMode (double signalSpread, double rate)
	: BaseTransmissionMode (signalSpread, rate)
{}
BPSKMode::~BPSKMode ()
{}

double 
BPSKMode::chunkSuccessRate (double snr, unsigned int nbits)
{
	double ber = BPSKBER (snr);
	double csr = pow (1 - ber, nbits);
	return csr;
}

BPSKFECMode::BPSKFECMode (unsigned int dFree, unsigned int adFree, 
			  double signalSpread, double rate, double codingRate)
	: FECBaseTransmissionMode (signalSpread, rate, codingRate),
	  dFree_ (dFree),
	  adFree_ (adFree)
{}
BPSKFECMode::~BPSKFECMode ()
{}
double
BPSKFECMode::chunkSuccessRate (double snr, unsigned int nbits)
{
	double ber = BPSKBER (snr);
	//printf ("%g\n", ber);
	if (ber == 0) {
		return 1;
	}
	/* only the first term */
	//printf ("dfree: %d, adfree: %d\n", dFree_, adFree_);
	double pd = calculatePd (ber, dFree_);
	double pmu = adFree_ * pd;
	double pms = pow (1 - pmu, nbits);
	//printf ("ber: %g -- pd: %g -- pmu: %g -- pms: %g\n", ber, pd, pmu, pms);
	return pms;
}
