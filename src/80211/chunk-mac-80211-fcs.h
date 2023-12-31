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
#ifndef CHUNK_MAC_80211_FCS_H
#define CHUNK_MAC_80211_FCS_H

#include "yans/chunk.h"
#include "yans/mac-address.h"
#include <stdint.h>

namespace yans {

class ChunkMac80211Fcs : public Chunk {
public:
	ChunkMac80211Fcs ();

	virtual ~ChunkMac80211Fcs ();

	uint32_t get_size (void) const;

private:
	virtual void print (std::ostream *os) const;
	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const *buffer);
	virtual void remove_from (Buffer *buffer);
};

}; // namespace yans

#endif /* CHUNK_MAC_80211_FCS_H */
