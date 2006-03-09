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

#ifndef REF_HOLDER_TCC
#define REF_HOLDER_TCC

#include "holder-traits.tcc"

namespace yans {

template <typename T>
class RefHolder {
public:
	typedef T *real_type;

	RefHolder (T *env) 
		: m_env (env) {
		m_env->ref ();
	}
	~RefHolder () {
		m_env->unref ();
	}
	RefHolder (RefHolder const&o) {
		m_env = o.m_env;
		m_env->ref ();
	}
	operator T *() const {
		return m_env;
	}
private:
	RefHolder &operator = (RefHolder const& o);
	T *m_env;
};

template <typename T>
struct HolderTraits<RefHolder<T> > {
  typedef RefHolder<T> held_type;
  typedef typename RefHolder<T>::real_type real_type;
};


template<typename T>
RefHolder<T>
make_ref_holder (T *t) {
	return RefHolder<T> (t);
}

}; // namespace yans

#endif /* REF_HOLDER_TCC */
