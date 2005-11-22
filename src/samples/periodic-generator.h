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

class Packet;

class GeneratorListener {
public:
	GeneratorListener ();
	virtual ~GeneratorListener () = 0;
	void ref (void);
	void unref (void);
	virtual void send (Packet *packet) = 0;
private:
	RefCount<GeneratorListener> m_ref;
};

class PeriodicGenerator {
public:
	PeriodicGenerator ();
	~PeriodicGenerator ();

	void set_listener (GeneratorListener *listener);

	void set_packet_interval (double interval);
	void set_packet_size (uint16_t size);

	void start_at (double start);
	void stop_at (double end);

private:
	friend class PeriodicGeneratorEvent;
	void send_next_packet (void);

	GeneratorListener *m_listener;
	double m_interval;
	uint16_t m_size;
	double m_stop_at;
};


#endif /* PERIODIC_GENERATOR_H */
