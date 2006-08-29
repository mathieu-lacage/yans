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
#ifndef CR_MAC_STATIONS_H
#define CR_MAC_STATIONS_H

#include "yans/stdint.h"

#include "mac-stations.h"
#include "mac-station.h"

namespace yans {

class CrMacStations : public MacStations {
public:
	CrMacStations (uint8_t data_mode, uint8_t ctl_mode);
	virtual ~CrMacStations ();

	uint8_t get_data_mode (void) const;
	uint8_t get_ctl_mode (void) const;
private:
	virtual class MacStation *create_station (void);

	uint8_t m_data_mode;
	uint8_t m_ctl_mode;
};


class CrMacStation : public MacStation
{
public:
	CrMacStation (CrMacStations *stations);
	virtual ~CrMacStation ();

	virtual void report_rx_ok (double snr, uint8_t mode);

	virtual void report_rts_failed (void);
	virtual void report_data_failed (void);
	virtual void report_rts_ok (double cts_snr, uint8_t cts_mode, uint8_t rts_snr);
	virtual void report_data_ok (double ack_snr, uint8_t ack_mode, uint8_t data_snr);
	virtual uint8_t get_data_mode (int size);
	virtual uint8_t get_rts_mode (void);
	virtual uint8_t snr_to_snr (double snr);
private:
	CrMacStations *m_stations;
};

}; // namespace yans



#endif /* CR_MAC_STATIONS_H */
