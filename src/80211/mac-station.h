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

namespace yans {

class MacStations;

class MacStation {
public:
	MacStation (MacStations *stations);

	virtual ~MacStation ();

	/* All stations are assumed to be 
	 * Authenticated so this method always
	 * returns true.
	 */
	bool is_associated (void) const;
	void record_associated (void);	

	// reception-related method
	virtual void report_rx_ok (double rx_snr, int tx_mode) = 0;

	// transmission-related methods
	virtual void report_rts_failed (void) = 0;
	virtual void report_data_failed (void) = 0;
	virtual void report_rts_ok (double cts_snr, int cts_mode) = 0;
	virtual void report_data_ok (double ack_snr, int ack_mode) = 0;
	virtual void report_final_rts_failed (void) = 0;
	virtual void report_final_data_failed (void) = 0;
	virtual int get_data_mode (int size) = 0;
	virtual int get_rts_mode (void) = 0;

protected:
	int get_n_modes (void) const;
private:
	bool m_is_associated;
	MacStations *m_stations;
};

}; // namespace yans

#endif /* MAC_STA_H */
