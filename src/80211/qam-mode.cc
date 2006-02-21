/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*-  *
 *
 * Copyright (c) 2004,2005,2006 INRIA
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

namespace yans {

NoFecQamMode::NoFecQamMode (double signalSpread, double rate, unsigned int M)
	: NoFecTransmissionMode (signalSpread, rate),
	  m_m (M)
{}
NoFecQamMode::~NoFecQamMode ()
{}

double 
NoFecQamMode::get_chunk_success_rate (double snr, unsigned int nbits) const
{
	double ber = get_qam_ber (snr, m_m);
	double csr = pow (1 - ber, nbits);
	return csr;
}

FecQamMode::FecQamMode (double signalSpread, double rate,
			double codingRate,
			unsigned int M, 
			unsigned int dFree, 
			unsigned int adFree,
			unsigned int adFreePlusOne)
	: FecTransmissionMode (signalSpread, rate, codingRate),
	  m_m (M), m_d_free (dFree), 
	  m_ad_free (adFree),
	  m_ad_free_plus_one (adFreePlusOne)
{}
FecQamMode::~FecQamMode ()
{}
double
FecQamMode::get_chunk_success_rate (double snr, unsigned int nbits) const
{
	double ber = get_qam_ber (snr, m_m);
	if (ber == 0.0) {
		return 1.0;
	}
	/* first term */
	double pd = calculate_pd (ber, m_d_free);
	double pmu = m_ad_free * pd;
	/* second term */
	pd = calculate_pd (ber, m_d_free + 1);
	pmu += m_ad_free_plus_one * pd;

	double pms = pow (1 - pmu, nbits);
	return pms;
}

}; // namespace yans
