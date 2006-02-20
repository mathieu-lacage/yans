/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 *  Copyright (C) 2001  Pierre L'Ecuyer (lecuyer@iro.umontreal.ca)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *  02110-1301 USA
 *
 */
#ifndef RNG_H_
#define RNG_H_

#include <stdint.h>


/*
 * Use class RNG in real programs.
 */
class RNG {
public:
	RNG ();

	void reset (long seed);


	uint32_t get_max (void) const;
	uint32_t get_min (void) const;
	uint32_t get_uint (void);
	uint32_t get_uint (uint32_t n);
	uint32_t get_uint (uint32_t low, uint32_t high);
	double get_double (void);

private:
	double U01 (); 
	double U01d (); 


	/*
	  Vectors to store the current seed, the beginning of the current block
	  (substream) and the beginning of the current stream.
	*/
	double Cg_[6], Bg_[6], Ig_[6]; 
}; 

#endif
