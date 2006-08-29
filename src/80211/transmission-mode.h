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

#ifndef TRANSMISSION_MODE_H
#define TRANSMISSION_MODE_H

#include "yans/stdint.h"

namespace yans {

class TransmissionMode {
public:
	virtual ~TransmissionMode () = 0;

	/* returns the half-width of the signal (Hz) 
	 */
	virtual double get_signal_spread (void) const = 0;

	/* returns the number of user bits per 
	 * second achieved by this transmission mode 
	 */
	virtual uint32_t get_data_rate (void) const = 0;

	/* returns the number of raw bits per
	 * second achieved by this transmission mode.
	 * the value returned by getRate and getDataRate
	 * will be different only if there is some
	 * FEC overhead.
	 */
	virtual uint32_t get_rate (void) const = 0;

	/* @snr: the snr, (W/W)
	 * @nbits: length of transmission (bits)
	 * 
	 * Returns the probability that nbits be successfully transmitted.
	 */
	virtual double get_chunk_success_rate (double snr, unsigned int nbits) const = 0;
};

class NoFecTransmissionMode : public TransmissionMode {
public:
	NoFecTransmissionMode (double signal_spread, uint32_t rate);
	virtual ~NoFecTransmissionMode () = 0;
	virtual double get_signal_spread (void) const;
	virtual uint32_t get_data_rate (void) const;
	virtual uint32_t get_rate (void) const;

private:
	double m_signal_spread;
	uint32_t m_rate;
protected:
	double get_bpsk_ber (double snr) const;
	double get_qam_ber (double snr, unsigned int m) const;
	double log2 (double v) const;
};

class FecTransmissionMode  : public NoFecTransmissionMode {
public:
	FecTransmissionMode (double signal_spread, uint32_t rate, double coding_rate);
	virtual ~FecTransmissionMode () = 0;
	virtual uint32_t get_data_rate (void) const;
protected:
	double calculate_pd_odd (double ber, unsigned int d) const;
	double calculate_pd_even (double ber, unsigned int d) const;
	double calculate_pd (double ber, unsigned int d) const;

private:
	uint32_t factorial (uint32_t k) const;
	double binomial (uint32_t k, double p, uint32_t n) const;
	double m_coding_rate;
};


}; // namespace yans


#endif /* TRANSMISSION_MODE_H */
