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

#include "event.h"
#include "event-id.h"
#include "simulator.h"
#include <iostream>

using namespace yans;

class MyEvent : public Event {
public:
        MyEvent ();
	virtual void notify (void);
};

MyEvent::MyEvent () 
{}

void
MyEvent::notify (void)
{
        std::cout << "notified at " << Simulator::now_s () << "s" << std::endl;
        //Simulator::insert_at_s (.0, new MyEvent ());
	delete this;
}

int main (int argc, char *argv[])
{
	EventId id;
        Simulator::insert_at_s (10.0, new MyEvent ());
        id = Simulator::insert_at_s (11.0, new MyEvent ());
        Simulator::insert_at_s (12.0, new MyEvent ());
        Simulator::insert_at_s (9.0, new MyEvent ());
        Simulator::insert_at_s (5.0, new MyEvent ());
        Simulator::insert_at_s (6.0, new MyEvent ());

	Simulator::remove (id);

        Simulator::insert_at_s (8.0, new MyEvent ());
        Simulator::insert_at_s (13.0, new MyEvent ());
        Simulator::insert_at_s (7.0, new MyEvent ());
        Simulator::insert_at_s (9.0, new MyEvent ());

	Simulator::run ();

	Simulator::destroy ();
}
