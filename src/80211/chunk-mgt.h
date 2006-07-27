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
#ifndef CHUNK_MGT_H
#define CHUNK_MGT_H

#include <stdint.h>

#include "chunk.h"
#include "status-code.h"
#include "capability-information.h"
#include "supported-rates.h"
#include "ssid.h"

namespace yans {

class Buffer;

class ChunkMgtAssocRequest : public Chunk {
public:
	ChunkMgtAssocRequest ();
	virtual ~ChunkMgtAssocRequest ();

	void set_ssid (Ssid ssid);
	void set_supported_rates (SupportedRates rates);
	void set_listen_interval (uint16_t interval);

	Ssid get_ssid (void) const;
	SupportedRates get_supported_rates (void) const;
	uint16_t get_listen_interval (void) const;

	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const*buffer);
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint32_t get_size (void) const;
	Ssid m_ssid;
	SupportedRates m_rates;
	CapabilityInformation m_capability;
	uint16_t m_listen_interval;
	uint8_t m_read_size;
};

class ChunkMgtAssocResponse : public Chunk {
public:
	ChunkMgtAssocResponse ();
	virtual ~ChunkMgtAssocResponse ();

	StatusCode get_status_code (void);
	void set_status_code (StatusCode code);

	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const*buffer);
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint32_t get_size (void) const;
	SupportedRates m_rates;
	CapabilityInformation m_capability;
	StatusCode m_code;
	uint16_t m_aid;
	uint8_t m_read_size;
};

class ChunkMgtProbeRequest : public Chunk {
public:
	virtual ~ChunkMgtProbeRequest ();

	void set_ssid (Ssid ssid);
	void set_supported_rates (SupportedRates rates);
	Ssid get_ssid (void) const;
	SupportedRates get_supported_rates (void) const;

	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const*buffer);
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint32_t get_size (void) const;
	Ssid m_ssid;
	SupportedRates m_rates;
	uint8_t m_read_size;
};

class ChunkMgtProbeResponse : public Chunk {
public:
  	ChunkMgtProbeResponse ();
	virtual ~ChunkMgtProbeResponse ();

	Ssid get_ssid (void) const;
	uint64_t get_beacon_interval_us (void) const;
	SupportedRates get_supported_rates (void) const;

	void set_ssid (Ssid ssid);
	void set_beacon_interval_us (uint64_t us);
	void set_supported_rates (SupportedRates rates);

	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const*buffer);
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
private:
	uint32_t get_size (void) const;
	Ssid m_ssid;
	uint64_t m_beacon_interval;
	SupportedRates m_rates;
	CapabilityInformation m_capability;
	uint8_t m_read_size;
};

class ChunkMgtBeacon : public ChunkMgtProbeResponse {};


}; // namespace yans

#endif /* CHUNK_MGT_H */
