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
#ifndef THROUGHPUT_PRINTER_H
#define THROUGHPUT_PRINTER_H

#include "yans/stdint.h"
#include "yans/event.h"
#include "yans/packet.h"

namespace yans {

class ThroughputPrinter {
public:
	ThroughputPrinter ();
	void set_print_interval_us (uint64_t us);
	void stop (void);
	void receive (Packet const packet);
private:
	void timeout (void);
	uint32_t m_current;
	uint32_t m_prev;
	uint64_t m_period_us;
	Event m_timeout_event;
};

}; // namespace yans

#endif /* THROUGHPUT_PRINTER_H */
