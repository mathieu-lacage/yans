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

#include "qam-mode.h"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>


QAMMode::QAMMode (unsigned int M, double signalSpread, double rate)
	: BaseTransmissionMode (signalSpread, rate),
	  M_ (M)
{}
QAMMode::~QAMMode ()
{}

double 
QAMMode::chunkSuccessRate (double snr, unsigned int nbits)
{
	double ber = QAMBER (snr, M_);
	double csr = pow (1 - ber, nbits);
	return csr;
}

QAMFECMode::QAMFECMode (unsigned int M, unsigned int dFree, 
			unsigned int adFree,
			unsigned int adFreePlusOne,
			double signalSpread, double rate,
			double codingRate)
	: FECBaseTransmissionMode (signalSpread, rate, codingRate),
	  M_ (M), dFree_ (dFree), 
	  adFree_ (adFree),
	  adFreePlusOne_ (adFreePlusOne)
{}
QAMFECMode::~QAMFECMode ()
{}
double
QAMFECMode::chunkSuccessRate (double snr, unsigned int nbits)
{
	double ber = QAMBER (snr, M_);
	if (ber == 0.0) {
		return 1.0;
	}
	/* first term */
	double pd = calculatePd (ber, dFree_);
	double pmu = adFree_ * pd;
	/* second term */
	pd = calculatePd (ber, dFree_ + 1);
	pmu += adFreePlusOne_ * pd;

	double pms = pow (1 - pmu, nbits);
	return pms;
}
