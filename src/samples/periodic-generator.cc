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

#include "periodic-generator.h"
#include "simulator.h"
#include "packet.h"
#include "chunk-fake-data.h"
#include "event.h"


GeneratorListener::GeneratorListener ()
	: m_ref (this)
{}
GeneratorListener::~GeneratorListener ()
{}
void 
GeneratorListener::ref (void)
{
	m_ref.ref ();
}
void 
GeneratorListener::unref (void)
{
	m_ref.unref ();
}



class PeriodicGeneratorEvent : public Event {
public:
	PeriodicGeneratorEvent (PeriodicGenerator *source);
	~PeriodicGeneratorEvent ();

	virtual void notify (void);
private:
	PeriodicGenerator *m_source;
};

PeriodicGeneratorEvent::PeriodicGeneratorEvent (PeriodicGenerator *source)
	: m_source (source)
{}
PeriodicGeneratorEvent::~PeriodicGeneratorEvent ()
{}
void 
PeriodicGeneratorEvent::notify (void)
{
	m_source->send_next_packet ();
	delete this;
}

PeriodicGenerator::PeriodicGenerator ()
{}

PeriodicGenerator::~PeriodicGenerator ()
{
	m_listener->unref ();
}


void 
PeriodicGenerator::set_listener (GeneratorListener *listener)
{
	listener->ref ();
	m_listener = listener;
}

void 
PeriodicGenerator::set_packet_interval (double interval)
{
	m_interval = interval;
}
void 
PeriodicGenerator::set_packet_size (uint16_t size)
{
	m_size = size;
}

void 
PeriodicGenerator::start_at (double start)
{
	Simulator::insert_at_s (start, new PeriodicGeneratorEvent (this));
}
void 
PeriodicGenerator::stop_at (double end)
{
	m_stop_at = end;
}


void 
PeriodicGenerator::send_next_packet (void)
{
	/* stop packet transmissions.*/
	if (m_stop_at > 0.0 && Simulator::now_s () >= m_stop_at) {
		return;
	}
	/* schedule next packet transmission. */
	Simulator::insert_in_s (m_interval, new PeriodicGeneratorEvent (this));
	/* create packet. */
	Packet *packet = new Packet ();
	ChunkFakeData *data = new ChunkFakeData (m_size);
	packet->add_header (data);
	m_listener->send (packet);
	packet->unref ();
}
