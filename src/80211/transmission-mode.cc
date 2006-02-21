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
#include <gsl_randist.h>

namespace yans {

TransmissionMode::~TransmissionMode ()
{}

NoFecTransmissionMode::NoFecTransmissionMode (double signal_spread, double rate)
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
double 
NoFecTransmissionMode::get_data_rate (void) const
{
	return m_rate;
}
double 
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



FecTransmissionMode::FecTransmissionMode (double signal_spread, double rate, double coding_rate)
	: NoFecTransmissionMode (signal_spread, rate),
	  m_coding_rate (coding_rate)
{}
FecTransmissionMode::~FecTransmissionMode ()
{}
double 
FecTransmissionMode::get_data_rate (void) const
{
	return m_rate * m_coding_rate;
}
double 
FecTransmissionMode::calculate_pd_odd (double ber, unsigned int d) const
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
double 
FecTransmissionMode::calculate_pd_even (double ber, unsigned int d) const
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
