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

#ifndef QAM_MODE_H
#define QAM_MODE_H

#include "transmission-mode.h"

namespace yans {

class NoFecQamMode : public NoFecTransmissionMode {
public:
	NoFecQamMode (double signal_spread, uint32_t rate, unsigned int m);
	virtual ~NoFecQamMode ();
	virtual double get_chunk_success_rate (double snr, unsigned int nbits) const;
private:
	unsigned int m_m;
};

class FecQamMode : public FecTransmissionMode {
public:
	FecQamMode (double signal_spread, 
		    uint32_t rate, 
		    double coding_rate,
		    unsigned int M, unsigned int d_free, 
		    unsigned int ad_free,
		    unsigned int ad_free_plus_one);
	virtual ~FecQamMode ();
	virtual double get_chunk_success_rate (double snr, unsigned int nbits) const;
private:
	unsigned int m_m;
	unsigned int m_d_free;
	unsigned int m_ad_free;
	unsigned int m_ad_free_plus_one;
};

}; // namespace yans


#endif /* QAM_MODE_H */
