/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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

#ifndef REF_COUNT_H
#define REF_COUNT_H

#include <stdint.h>

#define noTRACE_REF 1

#ifdef TRACE_REF
#include <iostream>
#include "simulator.h"
# define REF_TRACE(x) \
std::cout << "REF TRACE " << Simulator::now_s () << " " << x << std::endl;
#else /* TRACE_REF */
# define REF_TRACE(format,...)
#endif /* TRACE_REF */

namespace yans {

template<typename T>
class RefCount {
public:
	RefCount (T *obj)
	: m_ref (1), m_obj (obj) {
		REF_TRACE ("obj="<<m_obj<<", ref="<<m_ref);
	}
	~RefCount () {
		m_obj = (T *)0xdeadbeaf;
		m_ref = 0xdeadbeaf;
	}

	void ref (void) const {
		m_ref++;
		REF_TRACE ("obj="<<m_obj<<", ref="<<m_ref);
	}
	void unref (void) const {
		m_ref--;
		REF_TRACE ("obj="<<m_obj<<", ref="<<m_ref);
		if (m_ref == 0) {
			delete m_obj;
		}
	}
private:
	mutable uint32_t m_ref;
	T *m_obj;
};

#undef REF_TRACE
#undef TRACE_REF

}; // namespace yans

#endif /* REF_COUNT_H */
