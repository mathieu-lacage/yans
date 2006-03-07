/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#ifndef MAC_STATION_H
#define MAC_STATION_H

#include <stdint.h>

namespace yans {

class MacStations;

class MacStation {
public:
	MacStation ();

	virtual ~MacStation ();

	/* All stations are assumed to be 
	 * Authenticated so this method always
	 * returns true.
	 */
	bool is_associated (void) const;
	void record_associated (void);	

	// reception-related method
	virtual void report_rx_ok (double rx_snr, uint8_t tx_mode) = 0;

	// transmission-related methods
	virtual void report_rts_failed (void) = 0;
	virtual void report_data_failed (void) = 0;
	virtual void report_rts_ok (double cts_snr, uint8_t cts_mode, uint8_t rts_snr) = 0;
	virtual void report_data_ok (double ack_snr, uint8_t ack_mode, uint8_t data_snr) = 0;
	virtual void report_final_rts_failed (void) = 0;
	virtual void report_final_data_failed (void) = 0;
	virtual uint8_t get_data_mode (int size) = 0;
	virtual uint8_t get_rts_mode (void) = 0;
	virtual uint8_t snr_to_snr (double snr) = 0;

private:
	bool m_is_associated;
};

}; // namespace yans

#endif /* MAC_STATION_H */
