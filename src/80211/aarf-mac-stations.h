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
#ifndef AARF_MAC_STATIONS_H
#define AARF_MAC_STATIONS_H

#include "arf-mac-stations.h"

namespace yans {

class AarfMacStations : public ArfMacStations {
public:
	AarfMacStations ();
	virtual ~AarfMacStations ();
private:
	virtual class MacStation *createStation (void);
};

class AarfMacStation : public ArfMacStation
{
public:
	AarfMacStation (MacStations *stations,
			double success_k,
			int max_success_threshold,
			double timer_k);
	virtual ~AarfMacStation ();

private:
	virtual void report_recovery_failure (void);
	virtual void report_failure (void);

	double m_success_k;
        int m_max_success_threshold;
        double m_timer_k;
};

}; // namespace yans


#endif /* AARF_MAC_STATIONS_H */
