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

#ifndef _ns_transmission_mode_h_
#define _ns_transmission_mode_h_

#include <stdint.h>

class TransmissionMode
{
public:
	TransmissionMode () {}
	virtual ~TransmissionMode () {}

	/** 
	 * returns the half-width of the signal (Hz) 
	 */
	virtual double getSignalSpread (void) const = 0;

	/** 
	 * returns the number of physical bits per second achieved by this transmission mode 
	 */
	virtual double getRate (void) const = 0;

	/**
	 * @snr: the snr, (W/W)
	 * @nbits: length of transmission (bits)
	 * 
	 * Returns the probability that nbits be successfully transmitted.
	 */
	virtual double chunkSuccessRate (double snr, unsigned int nbits) = 0;

	/**
	   Maybe we need to add a new method which returns a unique id which identifies
	   the modulation type of this transmission mode.
	   XXX I am not sure: maybe we can use the rate to identify the tranmission mode
	   and deduce their modulation type. This sounds fragile though.
	   virtual int Modulation (void);
	 */
};

#endif // _ns_transmission_mode_h_
