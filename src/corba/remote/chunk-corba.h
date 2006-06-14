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

#ifndef CHUNK_CORBA_H
#define CHUNK_CORBA_H

#include "yans/chunk.h"
#include <ostream>
#include "remote-context.h"

class ChunkCorba : public yans::Chunk {
public:
	ChunkCorba (const ::Remote::Buffer& buffer);
	virtual ~ChunkCorba ();

	virtual void add_to (yans::Buffer *buffer) const;
	virtual void remove_from (yans::Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	const ::Remote::Buffer& m_buffer;
};

#endif /* CHUNK_CORBA_H */
