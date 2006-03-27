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
#ifndef CHUNK_PROBE_H
#define CHUNK_PROBE_H

#include "chunk.h"
#include "ssid.h"

namespace yans {

class ChunkProbeRequest : public Chunk {
public:
	ChunkProbeRequest ();
	virtual ~ChunkProbeRequest ();

	void set_ssid (Ssid ssid);
	Ssid get_ssid (void);
	void add_supported_rate (uint32_t bs);
	bool is_supported_rate (uint32_t bs);

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	Ssid m_ssid;
	uint8_t m_n_rates;
	uint8_t m_rates[8];
};

class ChunkProbeResponse : public Chunk {
public:
  	ChunkProbeResponse ();
	virtual ~ChunkProbeResponse ();

	void set_ssid (Ssid ssid);

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	Ssid m_ssid;
};

}; // namespace yans

#endif /* CHUNK_PROBE_H */
