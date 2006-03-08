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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "transmission-mode.h"

#include <math.h>
#include <cassert>

namespace yans {

TransmissionMode::~TransmissionMode ()
{}

NoFecTransmissionMode::NoFecTransmissionMode (double signal_spread, uint32_t rate)
	: m_signal_spread (signal_spread),
	  m_rate (rate)
{}
NoFecTransmissionMode::~NoFecTransmissionMode ()
{}
double 
NoFecTransmissionMode::get_signal_spread (void) const
{
	return m_signal_spread;
}
uint32_t
NoFecTransmissionMode::get_data_rate (void) const
{
	return m_rate;
}
uint32_t
NoFecTransmissionMode::get_rate (void) const
{
	return m_rate;
}
double 
NoFecTransmissionMode::log2 (double val) const
{
	return log(val) / log(2.0);
}
double 
NoFecTransmissionMode::get_bpsk_ber (double snr) const
{
	double EbNo = snr * m_signal_spread / m_rate;
	double z = sqrt(EbNo);
        double ber = 0.5 * erfc(z);
	return ber;
}
double 
NoFecTransmissionMode::get_qam_ber (double snr, unsigned int m) const
{
	double EbNo = snr * m_signal_spread / m_rate;
	double z = sqrt ((1.5 * log2 (m) * EbNo) / (m - 1.0));
        double z1 = ((1.0 - 1.0 / sqrt (m)) * erfc (z)) ;
        double z2 = 1 - pow ((1-z1), 2.0);
        double ber = z2 / log2 (m);
	return ber;
}



FecTransmissionMode::FecTransmissionMode (double signal_spread, uint32_t rate, double coding_rate)
	: NoFecTransmissionMode (signal_spread, rate),
	  m_coding_rate (coding_rate)
{}
FecTransmissionMode::~FecTransmissionMode ()
{}
uint32_t
FecTransmissionMode::get_data_rate (void) const
{
	return (uint32_t)(NoFecTransmissionMode::get_rate () * m_coding_rate);
}
uint32_t
FecTransmissionMode::factorial (uint32_t k) const
{
	uint32_t fact = 1;
	while (k > 0) {
		fact *= k;
		k--;
	}
	return fact;
}
double 
FecTransmissionMode::binomial (uint32_t k, double p, uint32_t n) const
{
	double retval = factorial (n) / (factorial (k) * factorial (n-k)) * pow (p, k) * pow (1-p, n-k);
	return retval;
}
double 
FecTransmissionMode::calculate_pd_odd (double ber, unsigned int d) const
{
	assert ((d % 2) == 1);
	unsigned int dstart = (d + 1) / 2;
	unsigned int dend = d;
	double pd = 0;

	for (unsigned int i = dstart; i < dend; i++) {
		pd += binomial (i, ber, d);
	}
	return pd;
}
double 
FecTransmissionMode::calculate_pd_even (double ber, unsigned int d) const
{
	assert ((d % 2) == 0);
	unsigned int dstart = d / 2 + 1;
	unsigned int dend = d;
	double pd = 0;

	for (unsigned int i = dstart; i < dend; i++){
                    pd +=  binomial (i, ber, d);
	}
	pd += 0.5 * binomial (d / 2, ber, d);

	return pd;
}

double 
FecTransmissionMode::calculate_pd (double ber, unsigned int d) const
{
	double pd;
	if ((d % 2) == 0) {
		pd = calculate_pd_even (ber, d);
	} else {
		pd = calculate_pd_odd (ber, d);
	}
	return pd;
}

}; // namespace yans
