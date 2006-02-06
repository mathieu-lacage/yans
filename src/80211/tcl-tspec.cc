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
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tcl-tspec.h"
#include "tcl-constructor.h"
#include "tspec.h"

#include <stdlib.h>

TCL_CONSTRUCTOR (TclTspec, "TclTspec");

TclTspec::TclTspec ()
	: m_tspec (new TSpec ())
{}

TSpec *
TclTspec::getTspec (void)
{
	return m_tspec;
}

int
TclTspec::command(int argc, const char*const* argv)
{
	if (argc == 3) {
		if (strcmp (argv[1], "set-traffic-type") == 0) {
			if (strcmp (argv[2], "periodic") == 0) {
				m_tspec->setTrafficType (TSpec::PERIODIC);
			} else if (strcmp (argv[2], "aperiodic") == 0) {
				m_tspec->setTrafficType (TSpec::APERIODIC);
			}
			return TCL_OK;
		} else if (strcmp (argv[1], "set-tsid") == 0) {
			int TSID = atoi (argv[2]);
			m_tspec->setTsid (TSID);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-link-direction") == 0) {
			enum TSpec::direction_e direction;
			if (strcmp (argv[2], "uplink") == 0) {
				direction = TSpec::UPLINK;
			} else if (strcmp (argv[2], "downlink") == 0) {
				direction = TSpec::DOWNLINK;
			} else if (strcmp (argv[2], "bidilink") == 0) {
				direction = TSpec::BIDILINK;
			} else if (strcmp (argv[2], "directlink") == 0) {
				direction = TSpec::DIRECTLINK;
			}
			m_tspec->setLinkDirection (direction);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-access-policy") == 0) {
			if (strcmp (argv[2], "HCCA") == 0) {
				m_tspec->setAccessPolicy (TSpec::HCCA);
			} else if (strcmp (argv[2], "EDCA") == 0) {
				m_tspec->setAccessPolicy (TSpec::EDCA);
			} else if (strcmp (argv[2], "HEMM") == 0) {
				m_tspec->setAccessPolicy (TSpec::HEMM);
			}
			return TCL_OK;
		} else if (strcmp (argv[1], "set-user-priority") == 0) {
			int up = atoi (argv[2]);
			m_tspec->setUserPriority (up);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-minimum-service-interval") == 0) {
			double SI = atof (argv[2]);
			m_tspec->setMinimumServiceInterval (SI);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-maximum-service-interval") == 0) {
			double SI = atof (argv[2]);
			m_tspec->setMaximumServiceInterval (SI);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-delay-bound") == 0) {
			double delayBound = atof (argv[2]);
			m_tspec->setDelayBound (delayBound);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-nominal-msdu-size") == 0) {
			int nominalMsduSize = atoi (argv[2]);
			m_tspec->setNominalMsduSize (nominalMsduSize);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-mean-data-rate") == 0) {
			double meanDataRate = atof (argv[2]);
			m_tspec->setMeanDataRate (meanDataRate);
			return TCL_OK;
		} else if (strcmp (argv[1], "set-peak-data-rate") == 0) {
			double peakDataRate = atof (argv[2]);
			m_tspec->setPeakDataRate (peakDataRate);
			return TCL_OK;
		}
	}
	// XXX: we could implement getting these values too. we will
	// see later if it is really needed.
	return TclObject::command(argc,argv);
}


