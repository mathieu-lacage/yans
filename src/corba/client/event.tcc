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
#ifndef YAPNS_EVENT_TCC
#define YAPNS_EVENT_TCC

#include "yans/event.tcc"

namespace yapns {

template<typename T>
Event *make_event(void (T::*f) (void), T* t) {
	return yans::make_event (f, t);
}
template<typename T, typename T1>
Event *make_event(void (T::*f) (T1), T* t, T1 a1) {
	return yans::make_event (f, t, a1);
}
template<typename T, typename T1, typename T2>
Event *make_event(void (T::*f) (T1, T2), T* t, T1 a1, T2 a2) {
	return yans::make_event (f, t, a1, a2);
}
template<typename T, typename T1, typename T2, typename T3>
Event *make_event(void (T::*f) (T1, T2, T3), T* t, T1 a1, T2 a2, T3 a3) {
	return yans::make_event (f, t, a1, a2, a3);
}
template<typename T, typename T1, typename T2, typename T3, typename T4>
Event *make_event(void (T::*f) (T1, T2, T3, T4), T* t, T1 a1, T2 a2, T3 a3, T4 a4) {
	return yans::make_event (f, t, a1, a2, a3, a4);
}
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
Event *make_event(void (T::*f) (T1, T2, T3, T4, T5), T* t, T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) {
	return yans::make_event (f, t, a1, a2, a3, a4, a5);
}


}; // namespace yapns

#endif /* YAPNS_EVENT_TCC */
