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

#include "bpsk-mode.h"

#include <math.h>

namespace yans {

NoFecBpskMode::NoFecBpskMode (double signal_spread, uint32_t rate)
	: NoFecTransmissionMode (signal_spread, rate)
{}
NoFecBpskMode::~NoFecBpskMode ()
{}

double 
NoFecBpskMode::get_chunk_success_rate (double snr, unsigned int nbits) const
{
	double ber = get_bpsk_ber (snr);
	double csr = pow (1 - ber, nbits);
	return csr;
}

FecBpskMode::FecBpskMode (double signal_spread, uint32_t rate, double coding_rate, 
			  unsigned int d_free, unsigned int ad_free)
	: FecTransmissionMode (signal_spread, rate, coding_rate),
	  m_d_free (d_free),
	  m_ad_free (ad_free)
{}
FecBpskMode::~FecBpskMode ()
{}
double
FecBpskMode::get_chunk_success_rate (double snr, unsigned int nbits) const
{
	double ber = get_bpsk_ber (snr);
	//printf ("%g\n", ber);
	if (ber == 0) {
		return 1;
	}
	/* only the first term */
	//printf ("dfree: %d, adfree: %d\n", dFree_, adFree_);
	double pd = calculate_pd (ber, m_d_free);
	double pmu = m_ad_free * pd;
	if (pmu > 1.0) {
		/**
		 * If pmu is bigger than 1, then, this calculation is
		 * giving us a useless bound. A better bound in this case
		 * is 1 - ber which is necessarily bigger than the real 
		 * success rate.
		 */
		return ber;
	}
	double pms = pow (1 - pmu, nbits);
	//printf ("ber: %g -- pd: %g -- pmu: %g -- pms: %g\n", ber, pd, pmu, pms);
	return pms;
}

}; // namespace yans
