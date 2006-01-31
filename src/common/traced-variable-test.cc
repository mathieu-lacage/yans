/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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

#include "traced-variable-test.h"
#include "traced-variable.h"

namespace yans {

bool 
TracedVariableTest::run_tests (void)
{
	TracedVariable var, ovar;
	var = 10;
	ovar = var;

	if (var == ovar) {
	}
	if (var != ovar) {
	}
	if (var > ovar) {
	}
	if (var >= ovar) {
	}
	if (var < ovar) {
	}
	if (var <= ovar) {
	}

	if (var == 1) {
	}
	if (var != 1) {
	}
	if (var > 1) {
	}
	if (var >= 1) {
	}
	if (var < 1) {
	}
	if (var <= 1) {
	}

	if (1 == ovar) {
	}
	if (1 != ovar) {
	}
	if (1 > ovar) {
	}
	if (1 >= ovar) {
	}
	if (1 < ovar) {
	}
	if (1 <= ovar) {
	}

	var++;
	++var;
	var--;
	--var;

	TracedVariable tmp;
	tmp = var + ovar;
	tmp = var - ovar;
	tmp = var / ovar;
	tmp = var * ovar;
	tmp = var << ovar;
	tmp = var >> ovar;
	tmp = var & ovar;
	tmp = var | ovar;
	tmp = var ^ ovar;

	tmp = var + 1;
	tmp = var - 1;
	tmp = var / 1;
	tmp = var * 1;
	tmp = var << 1;
	tmp = var >> 1;
	tmp = var & 1;
	tmp = var | 1;
	tmp = var ^ 1;

	tmp = 1 + ovar;
	tmp = 1 - ovar;
	tmp = 1 / ovar;
	tmp = 1 * ovar;
	tmp = 1 << ovar;
	tmp = 1 >> ovar;
	tmp = 1 & ovar;
	tmp = 1 | ovar;
	tmp = 1 ^ ovar;

	tmp += var;
	tmp -= var;
	tmp /= var;
	tmp *= var;
	tmp <<= var;
	tmp >>= var;
	tmp &= var;
	tmp |= var;
	tmp ^= var;

	tmp += 1;
	tmp -= 1;
	tmp /= 1;
	tmp *= 1;
	tmp <<= 1;
	tmp >>= 1;
	tmp &= 1;
	tmp |= 1;
	tmp ^= 1;


	uint32_t utmp;
	utmp = var + ovar;
	utmp = var - ovar;
	utmp = var / ovar;
	utmp = var * ovar;
	utmp = var << ovar;
	utmp = var >> ovar;
	utmp = var & ovar;
	utmp = var | ovar;
	utmp = var ^ ovar;

	utmp = var + 1;
	utmp = var - 1;
	utmp = var / 1;
	utmp = var * 1;
	utmp = var << 1;
	utmp = var >> 1;
	utmp = var & 1;
	utmp = var | 1;
	utmp = var ^ 1;

	utmp = 1 + ovar;
	utmp = 1 - ovar;
	utmp = 1 / ovar;
	utmp = 1 * ovar;
	utmp = 1 << ovar;
	utmp = 1 >> ovar;
	utmp = 1 & ovar;
	utmp = 1 | ovar;
	utmp = 1 ^ ovar;

	utmp += var;
	utmp -= var;
	utmp /= var;
	utmp *= var;
	utmp <<= var;
	utmp >>= var;
	utmp &= var;
	utmp |= var;
	utmp ^= var;

	utmp += 1;
	utmp -= 1;
	utmp /= 1;
	utmp *= 1;
	utmp <<= 1;
	utmp >>= 1;
	utmp &= 1;
	utmp |= 1;
	utmp ^= 1;

	


	return true;
}

}; // namespace yans
