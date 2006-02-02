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

#ifndef F_TRACED_VARIABLE_TCC
#define F_TRACED_VARIABLE_TCC

#include "callback.tcc"
#include <stdint.h>
#include <cassert>

namespace yans {

class FTracedVariableBase {
public:
	typedef Callback<void (double, double)> ChangeNotifyCallback;

	FTracedVariableBase ()
		: m_callback (0) {}
	FTracedVariableBase (FTracedVariableBase const &o)
		: m_callback (0) {}
	FTracedVariableBase &operator = (FTracedVariableBase const &o) {
		return *this;
	}

	~FTracedVariableBase () {
		delete m_callback;
		m_callback = (ChangeNotifyCallback *)0xdeadbeaf;
	}

	void set_callback(ChangeNotifyCallback *callback) {
		assert (m_callback == 0);
		m_callback = callback;
	}
protected:
	void notify (double old_val, double new_val) {
		if (old_val != new_val && m_callback != 0) {
			(*m_callback) (old_val, new_val);
		}
	}
private:
	ChangeNotifyCallback *m_callback;
};


}; // namespace yans

#endif /* F_TRACED_VARIABLE_TCC */