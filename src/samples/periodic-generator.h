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

#ifndef PERIODIC_GENERATOR_H
#define PERIODIC_GENERATOR_H

#include <stdint.h>
#include "ref-count.tcc"

class Source;

class PeriodicGenerator {
public:
	PeriodicGenerator ();
	~PeriodicGenerator ();
	void ref (void);
	void unref (void);

	void set_source (Source *source);

	void set_packet_interval (double interval);
	void set_packet_size (uint16_t size);

	void start_at (double start);
	void stop_at (double end);

private:
	friend class PeriodicGeneratorEvent;
	void send_next_packet (void);

	Source *m_source;
	double m_interval;
	uint16_t m_size;
	double m_stop_at;
	RefCount<PeriodicGenerator> m_ref;
};


#endif /* PERIODIC_GENERATOR_H */