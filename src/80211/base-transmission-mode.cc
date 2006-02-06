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

#include "base-transmission-mode.h"

#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <assert.h>

BaseTransmissionMode::BaseTransmissionMode (double signalSpread, double rate)
	: TransmissionMode (),
	  signalSpread_ (signalSpread),
	  rate_ (rate)
{}

double BaseTransmissionMode::log2 (double val)
{
	return log(val) / log(2.0);
}


double BaseTransmissionMode::BPSKBER (double snr)
{
	double EbNo = snr * getSignalSpread () / getRate ();
	double z = sqrt(EbNo);
        double ber = 0.5 * erfc(z);
	return ber;
}

double BaseTransmissionMode::QAMBER (double snr, unsigned int M)
{
	double EbNo = snr * getSignalSpread () / getRate ();
	double z = sqrt ((1.5 * log2 (M) * EbNo) / (M - 1.0));
        double z1 = ((1.0 - 1.0 / sqrt (M)) * erfc (z)) ;
        double z2 = 1 - pow ((1-z1), 2.0);
        double ber = z2 / log2 (M);
	return ber;
}



FECBaseTransmissionMode::FECBaseTransmissionMode (double signalSpread, double rate)
	: BaseTransmissionMode (signalSpread, rate) 
{}
FECBaseTransmissionMode::~FECBaseTransmissionMode ()
{}
double FECBaseTransmissionMode::calculatePdOdd (double ber, unsigned int d)
{
	assert ((d % 2) == 1);
	unsigned int dstart = (d + 1) / 2;
	unsigned int dend = d;
	double pd = 0;

	for (unsigned int i = dstart; i < dend; i++){
                    pd +=  gsl_ran_binomial_pdf (i, ber, d);
	}
	return pd;
}
double FECBaseTransmissionMode::calculatePdEven (double ber, unsigned int d)
{
	assert ((d % 2) == 0);
	unsigned int dstart = d / 2 + 1;
	unsigned int dend = d;
	double pd = 0;

	for (unsigned int i = dstart; i < dend; i++){
                    pd +=  gsl_ran_binomial_pdf (i, ber, d);
	}
	pd += 0.5 * gsl_ran_binomial_pdf (d / 2, ber, d);

	return pd;
}

double FECBaseTransmissionMode::calculatePd (double ber, unsigned int d)
{
	double pd;
	if ((d % 2) == 0) {
		pd = calculatePdEven (ber, d);
	} else {
		pd = calculatePdOdd (ber, d);
	}
	return pd;
}
