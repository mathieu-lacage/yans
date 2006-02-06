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



FECBaseTransmissionMode::FECBaseTransmissionMode (double signalSpread, double rate, double codingRate)
	: BaseTransmissionMode (signalSpread, rate),
	  m_codingRate (codingRate)
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
