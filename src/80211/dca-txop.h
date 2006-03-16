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

#ifndef DCA_TXOP_H
#define DCA_TXOP_H

#include <stdint.h>

class Dcf;
class MacQueue80211e;
class Packet;
class MacLowTransmissionListener;
class MacStation;
class NetInterface80211;
class MacLow;
class MacParameters;

class DcaTxop 
{
public:
	DcaTxop (Dcf *dcf, MacQueue80211e *queue);

	void set_interface (NetInterface80211 *interface);

private:
	friend class MyDcaAccessListener;
	friend class MyDcaTransmissionListener;

	double now (void);

	MacLow *low (void);
	MacParameters *parameters (void);

	/* event handlers */
	void access_granted_now (void);
	bool accessing_and_will_notify (void);
	bool access_needed (void);
	void got_cts (double snr, int txMode);
	void missed_cts (void);
	void got_ack (double snr, int txMode);
	void missed_ack (void);
	void start_next (void);

	bool need_rts (void);
	bool need_fragmentation (void);
	int get_nfragments (void);
	int get_last_fragment_size (void);
	int get_next_fragment_size (void);
	int get_fragment_size (void);
	bool is_last_fragment (void);
	void next_fragment (void);
	Packet *getFragmentPacket (void);
	void drop_current_packet (void);
	MacStation *lookupDestStation (Packet *packet);

	Dcf *m_dcf;
	MacQueue80211e *m_queue;
	NetInterface80211 *m_interface;
	Packet *m_currentTxPacket;
	MacLowTransmissionListener *m_transmissionListener;
	int m_SSRC;
	int m_SLRC;
	int m_fragmentNumber;
};



#endif /* DCA_TXOP_H */
