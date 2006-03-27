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
#include "supported-rates.h"
#include "capability-information.h"

namespace yans {

class ChunkProbeRequest : public Chunk {
public:
	virtual ~ChunkProbeRequest ();

	void set_ssid (Ssid ssid);
	void set_supported_rates (SupportedRates rates);
	Ssid get_ssid (void) const;
	SupportedRates get_supported_rates (void) const;

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	Ssid m_ssid;
	SupportedRates m_rates;
};

class ChunkProbeResponse : public Chunk {
public:
  	ChunkProbeResponse ();
	virtual ~ChunkProbeResponse ();

	Ssid get_ssid (void) const;
	uint64_t get_beacon_interval_us (void) const;
	SupportedRates get_supported_rates (void) const;

	void set_ssid (Ssid ssid);
	void set_beacon_interval_us (uint64_t us);
	void set_supported_rates (SupportedRates rates);

	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	Ssid m_ssid;
	uint64_t m_beacon_interval;
	SupportedRates m_rates;
	CapabilityInformation m_capability;
};

}; // namespace yans

#endif /* CHUNK_PROBE_H */
