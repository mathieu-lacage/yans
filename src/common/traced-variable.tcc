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
#include <stdint.h>
#include <cassert>

namespace yans {

class UiTracedVariableBase {
public:
	typedef Callback<void (uint64_t, uint64_t)> ChangeNotifyCallback;

	UiTracedVariableBase ()
		: m_callback (0) {}

	~UiTracedVariableBase () {
		if (m_callback != 0) {
			m_callback = (ChangeNotifyCallback *)0xdeadbeaf;
			delete m_callback;
		}
	}

	void set_callback(ChangeNotifyCallback *callback) {
		assert (m_callback == 0);
		m_callback = callback;
	}
protected:
	void notify (uint64_t old_val, uint64_t new_val) {
		if (m_callback != 0) {
			(*m_callback) (old_val, new_val);
		}
	}
private:
	ChangeNotifyCallback *m_callback;
};
class SiTracedVariableBase {
public:
	typedef Callback<void (int64_t, int64_t)> ChangeNotifyCallback;

	SiTracedVariableBase ()
		: m_callback (0) {}

	~SiTracedVariableBase () {
		if (m_callback != 0) {
			m_callback = (ChangeNotifyCallback *)0xdeadbeaf;
			delete m_callback;
		}
	}

	void set_callback(ChangeNotifyCallback *callback) {
		assert (m_callback == 0);
		m_callback = callback;
	}
protected:
	void notify (int64_t old_val, int64_t new_val) {
		if (m_callback != 0) {
			(*m_callback) (old_val, new_val);
		}
	}
private:
	ChangeNotifyCallback *m_callback;
};
class FTracedVariableBase {
public:
	typedef Callback<void (double, double)> ChangeNotifyCallback;

	FTracedVariableBase ()
		: m_callback (0) {}

	~FTracedVariableBase () {
		if (m_callback != 0) {
			m_callback = (ChangeNotifyCallback *)0xdeadbeaf;
			delete m_callback;
		}
	}

	void set_callback(ChangeNotifyCallback *callback) {
		assert (m_callback == 0);
		m_callback = callback;
	}
protected:
	void notify (double old_val, double new_val) {
		if (m_callback != 0) {
			(*m_callback) (old_val, new_val);
		}
	}
private:
	ChangeNotifyCallback *m_callback;
};


template <typename T>
class UiTracedVariable : public UiTracedVariableBase {
public:
	UiTracedVariable ()
		: m_var (0)
	{}
	UiTracedVariable (T const &var) 
		: m_var (var)
	{}


	UiTracedVariable &operator = (UiTracedVariable const &o) {
		assign (o.m_var);
		return *this;
	}
	UiTracedVariable &operator++ () {
		m_var++;
		return *this;
	}
	UiTracedVariable &operator-- () {
		m_var--;
		return *this;
	}
	UiTracedVariable operator++ (int) {
		UiTracedVariable old (*this);
		++*this;
		return old;
	}
	UiTracedVariable operator-- (int) {
		UiTracedVariable old (*this);
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
UiTracedVariable<T> &operator += (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () + rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator -= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () - rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator *= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () * rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator /= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () / rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator <<= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () << rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator >>= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () >> rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator &= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () & rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator |= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () | rhs.get ());
	return lhs;
}
template <typename T>
UiTracedVariable<T> &operator ^= (UiTracedVariable<T> &lhs, UiTracedVariable<T> const &rhs) {
	lhs.assign (lhs.get () ^ rhs.get ());
	return lhs;
}


template <typename T, typename U>
UiTracedVariable<T> &operator += (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () + rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator -= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () - rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator *= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () * rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator /= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () / rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator <<= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () << rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator >>= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () >> rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator &= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () & rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator |= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () | rhs);
	return lhs;
}
template <typename T, typename U>
UiTracedVariable<T> &operator ^= (UiTracedVariable<T> &lhs, U const &rhs) {
	lhs.assign (lhs.get () ^ rhs);
	return lhs;
}

}; // namespace yans

#endif /* TRACED_VARIABLE_TCC */
