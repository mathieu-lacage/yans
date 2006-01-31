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

#ifndef TRACED_VARIABLE_TCC
#define TRACED_VARIABLE_TCC

#include "callback.tcc"

namespace yans {

template <typename T>
class TracedVariable {
public:
	typedef Callback<void (T, T)> ChangeNotifyCallback;

	TracedVariable ()
		: m_var (0),
		  m_callback (0) 
	{}
	TracedVariable (T const &var) 
		: m_var (var),
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
	operator T () const {
		return get ();
	}


	void assign (T var) {
		if (m_var != var && m_callback != 0) {
			(*m_callback) (m_var, var);
		}
		m_var = var;
	}
        T get (void) const {
		return m_var;
	}

private:
	T m_var;
	ChangeNotifyCallback *m_callback;
};

template <typename T>
TracedVariable<T> &operator += (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () + rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator -= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () - rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator *= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () * rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator /= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () / rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator <<= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () << rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator >>= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () >> rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator &= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () & rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator |= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () | rhs.get ());
	return lhs;
}
template <typename T>
TracedVariable<T> &operator ^= (TracedVariable<T> &lhs, TracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () ^ rhs.get ());
	return lhs;
}


template <typename T, typename U>
TracedVariable<T> &operator += (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () + rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator -= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () - rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator *= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () * rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator /= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () / rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator <<= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () << rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator >>= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () >> rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator &= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () & rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator |= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () | rhs);
	return lhs;
}
template <typename T, typename U>
TracedVariable<T> &operator ^= (TracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () ^ rhs);
	return lhs;
}

}; // namespace yans

#endif /* TRACED_VARIABLE_TCC */
