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

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <ostream>

namespace yans {

class Buffer;

/**
 * \brief Protocol header serialization and deserialization.
 *
 * Every Protocol header which needs to be inserted and removed
 * from a Packet instance must derive from this abstract base class
 * and implement the pure virtual methods defined here.
 */
class Chunk {
public:
	/**
	 * Derived classes must provided an explicit virtual destructor
	 */
	virtual ~Chunk () = 0;

	/**
	 * \param buffer the buffer in which the protocol header
	 *        must serialize itself.
	 */
	virtual void add_to (Buffer *buffer) const = 0;
	/**
	 * \param buffer the buffer from which the protocol header
	 *        must deserialize itself.
	 */
	virtual void remove_from (Buffer *buffer) = 0;
	/**
	 * \param os the std output stream in which this 
	 *           protocol header must print itself.
	 */
	virtual void print (std::ostream *os) const = 0;
};

std::ostream& operator<< (std::ostream& os, Chunk const& chunk);

}; // namespace yans

#endif /* CHUNK_H */
