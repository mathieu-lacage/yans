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

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "yans/stdint.h"
#include <list>

namespace yans {

class Fiber;

class Semaphore {
public:
	Semaphore (int32_t n);
	~Semaphore ();

	void up (void);
	void down (void);

	void up (uint8_t delta);
	void down (uint8_t delta);

	void up_all (void);
	void down_all (void);
private:
	int32_t m_n;
	std::list<Fiber *> m_waiting;
};

}; // namespace yans


#endif /* SEMAPHORE_H */
