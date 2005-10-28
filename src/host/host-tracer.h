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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#ifndef HOST_TRACER_H
#define HOST_TRACER_H

class Host;
class Packet;
class WriteFile;

class HostTracer {
public:
	void trace_tx_app  (Packet *packet);
	void trace_tx_udp  (Packet *packet);
	void trace_tx_ipv4 (Packet *packet);

	void trace_rx_app  (Packet *packet);
	void trace_rx_udp  (Packet *packet);
	void trace_rx_ipv4 (Packet *packet);

	void enable_all (void);
	void disable_all (void);
private:
	friend class Host;
	HostTracer (WriteFile *file);
	~HostTracer ();

	bool m_enable_all;
	bool m_enable_app;
	bool m_enable_udp;
	bool m_enable_ipv4;
	WriteFile *m_file;
};

#endif /* HOST_TRACER_H */
