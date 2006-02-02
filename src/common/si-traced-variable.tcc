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

#ifndef SI_TRACED_VARIABLE_TCC
#define SI_TRACED_VARIABLE_TCC

#include "callback.tcc"
#include <stdint.h>
#include <cassert>

namespace yans {

class SiTracedVariableBase {
public:
	typedef Callback<void (int64_t, int64_t)> ChangeNotifyCallback;

	SiTracedVariableBase ()
		: m_callback (0) {}
	SiTracedVariableBase (SiTracedVariableBase const &o)
		: m_callback (0) {}
	SiTracedVariableBase &operator = (SiTracedVariableBase const &o) {
		return *this;
	}

	~SiTracedVariableBase () {
		delete m_callback;
		m_callback = (ChangeNotifyCallback *)0xdeadbeaf;
	}

	void set_callback(ChangeNotifyCallback *callback) {
		assert (m_callback == 0);
		m_callback = callback;
	}
protected:
	void notify (int64_t old_val, int64_t new_val) {
		if (old_val != new_val && m_callback != 0) {
			(*m_callback) (old_val, new_val);
		}
	}
private:
	ChangeNotifyCallback *m_callback;
};

template <typename T>
class UiTracedVariable;


template <typename T>
class SiTracedVariable : public SiTracedVariableBase {
public:
	SiTracedVariable ()
		: m_var (0)
	{}
	SiTracedVariable (T const &var) 
		: m_var (var)
	{}


	template <typename TT>
	SiTracedVariable &operator = (SiTracedVariable<TT> const &o) {
		assign (o.get ());
		return *this;
	}
	template <typename TT>
	SiTracedVariable &operator = (UiTracedVariable<TT> const &o) {
		assign (o.get ());
		return *this;
	}
	SiTracedVariable &operator++ () {
		m_var++;
		return *this;
	}
	SiTracedVariable &operator-- () {
		m_var--;
		return *this;
	}
	SiTracedVariable operator++ (int) {
		SiTracedVariable old (*this);
		++*this;
		return old;
	}
	SiTracedVariable operator-- (int) {
		SiTracedVariable old (*this);
		--*this;
		return old;
	}
	operator T () const {
		return get ();
	}


	void assign (T var) {
		notify (m_var, var);
		m_var = var;
	}
        T get (void) const {
		return m_var;
	}

private:
	T m_var;
};

template <typename T>
SiTracedVariable<T> &operator += (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () + rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator -= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () - rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator *= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () * rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator /= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () / rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator <<= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () << rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator >>= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () >> rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator &= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () & rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator |= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () | rhs.get ());
	return lhs;
}
template <typename T>
SiTracedVariable<T> &operator ^= (SiTracedVariable<T> &lhs, SiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () ^ rhs.get ());
	return lhs;
}


template <typename T, typename U>
SiTracedVariable<T> &operator += (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () + rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator -= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () - rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator *= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () * rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator /= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () / rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator <<= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () << rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator >>= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () >> rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator &= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () & rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator |= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () | rhs);
	return lhs;
}
template <typename T, typename U>
SiTracedVariable<T> &operator ^= (SiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () ^ rhs);
	return lhs;
}

}; // namespace yans

#endif /* TRACED_VARIABLE_TCC */
