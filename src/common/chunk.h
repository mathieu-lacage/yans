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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#define CHUNK_H

#include <stdint.h>
#include <ostream>

class WriteBuffer;
class ReadBuffer;

class Chunk {
public:
	virtual ~Chunk () = 0;

	virtual uint32_t get_size (void) = 0;
	virtual Chunk *copy (void) = 0;
	virtual void serialize (WriteBuffer *buffer) = 0;
	virtual void deserialize (ReadBuffer *buffer) = 0;
	virtual void print (std::ostream *os) = 0;
};

#endif /* CHUNK_H */
