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

#ifndef TRACED_VARIABLE_H
#define TRACED_VARIABLE_H

#include "callback.tcc"

namespace yans {

class TracedVariable {
public:
	typedef Callback<void (uint32_t, uint32_t)> ChangeNotifyCallback;

	TracedVariable ()
		: m_var (0),
		  m_callback (0) 
	{}
	TracedVariable (uint32_t var) 
		: m_var (0),
		  m_callback (0)
	{}

	~TracedVariable () {
		if (m_callback != 0) {
			m_callback = (ChangeNotifyCallback *)0xdeadbeaf;
			delete m_callback;
		}
	}

	void set_callback(ChangeNotifyCallback *callback) {
		assert (m_callback == 0);
		m_callback = callback;
	}

	TracedVariable &operator = (TracedVariable const &o) {
		assign (o.m_var);
		return *this;
	}
	TracedVariable &operator++ () {
		m_var++;
		return *this;
	}
	TracedVariable &operator-- () {
		m_var--;
		return *this;
	}
	TracedVariable operator++ (int) {
		TracedVariable old (*this);
		++*this;
		return old;
	}
	TracedVariable operator-- (int) {
		TracedVariable old (*this);
		--*this;
		return old;
	}
	operator uint32_t () const {
		return get ();
	}


	void assign (uint32_t var) {
		if (m_var != var && m_callback != 0) {
			(*m_callback) (m_var, var);
		}
		m_var = var;
	}
	uint32_t get (void) const {
		return m_var;
	}

private:
	uint32_t m_var;
	ChangeNotifyCallback *m_callback;
};


TracedVariable &operator += (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () + rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator -= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () - rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator *= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () * rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator /= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () / rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator <<= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () << rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator >>= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () >> rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator &= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () & rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator |= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () | rhs.get ();
	lhs.assign (new_var);
	return lhs;
}
TracedVariable &operator ^= (TracedVariable &lhs, TracedVariable const &rhs) {
	uint32_t new_var =  lhs.get () ^ rhs.get ();
	lhs.assign (new_var);
	return lhs;
}

}; // namespace yans

#endif /* TRACED_VARIABLE_H */
