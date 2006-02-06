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

#include "tspec.h"

#include <tclcl.h>
#include <stdlib.h>


static class TSpecClass: public TclClass {
public:
        TSpecClass() : TclClass("TSPEC") {}
        TclObject* create(int, const char*const*) {
                return (new TSpec ());
        }
} class_TSpec;



TSpec::TSpec ()
{
	initialize ("nominalMSDUSize", &m_nominalMSDUSize, 1500);
	initialize ("maximalMSDUSize", &m_maximalMSDUSize, 1500);
	initialize ("minimumServiceInterval", &m_minimumServiceInterval, 0.0);
	initialize ("maximumServiceInterval", &m_maximumServiceInterval, 1.0);
	initialize ("inactivityInterval", &m_inactivityInterval, 1.0);
	initialize ("suspensionInterval", &m_suspensionInterval, 1.0);
	initialize ("serviceStartTime", &m_serviceStartTime, 1.0);
	initialize ("minimumDataRate", &m_minimumDataRate, 100000);
	initialize ("meanDataRate", &m_meanDataRate, 100000);
	initialize ("peakDataRate", &m_peakDataRate, 100000);
	initialize ("burstSize", &m_burstSize, 4000);
	initialize ("delayBound", &m_delayBound, 0.0);
	initialize ("minimumPhyMode", &m_minimumPhyMode, 0);
	initialize ("surplusBandwidthAllowance", &m_surplusBandwidthAllowance, 1.5);
	initialize ("mediumTime", &m_mediumTime, 1.0);
}

void
TSpec::initialize (char const *varName, uint32_t *variable, uint32_t defaultValue)
{
	if (isDefined (varName)) {
		bind (varName, variable);
	} else {
		*variable = defaultValue;
	}
}
void
TSpec::initialize (char const *varName, double *variable, double defaultValue)
{
	if (isDefined (varName)) {
		bind (varName, variable);
	} else {
		*variable = defaultValue;
	}
}


bool
TSpec::isDefined (char const *varName)
{
	Tcl& tcl=Tcl::instance();
	tcl.evalf ("catch \"TSPEC set %s\" val", varName);
	if (strcmp ((char *)tcl.result (), "0") == 0) {
		return true;
	} else {
		return false;
	}
}

int
TSpec::command(int argc, const char*const* argv)
{
	if (argc == 3) {
		if (strcmp (argv[1], "trafficType") == 0) {
			if (strcmp (argv[2], "periodic") == 0) {
				m_trafficType = PERIODIC;
			} else if (strcmp (argv[2], "aperiodic") == 0) {
				m_trafficType = APERIODIC;
			}
			return TCL_OK;
		} else if (strcmp (argv[1], "TSID") == 0) {
			m_TSID = atoi (argv[2]);
			return TCL_OK;
		} else if (strcmp (argv[1], "linkDirection") == 0) {
			if (strcmp (argv[2], "uplink") == 0) {
				m_direction = UPLINK;
			} else if (strcmp (argv[2], "downlink") == 0) {
				m_direction = DOWNLINK;
			} else if (strcmp (argv[2], "bidilink") == 0) {
				m_direction = BIDILINK;
			} else if (strcmp (argv[2], "directlink") == 0) {
				m_direction = DIRECTLINK;
			}
			return TCL_OK;
		} else if (strcmp (argv[1], "accessPolicy") == 0) {
			if (strcmp (argv[2], "HCCA") == 0) {
				m_accessPolicy = HCCA;
			} else if (strcmp (argv[2], "EDCA") == 0) {
				m_accessPolicy = EDCA;
			} else if (strcmp (argv[2], "HEMM") == 0) {
				m_accessPolicy = HEMM;
			}
			return TCL_OK;
		} else if (strcmp (argv[1], "userPriority") == 0) {
			m_UP = atoi (argv[2]);
			return TCL_OK;
		}
	}
	// XXX: we could implement getting these values too. we will
	// see later if it is really needed.
	return TclObject::command(argc,argv);
}


enum trafficType_e
TSpec::getTrafficType (void) const
{
	return m_trafficType;
}
uint8_t 
TSpec::getTSID (void) const
{
	return m_TSID;
}
enum direction_e 
TSpec::getLinkDirection (void) const
{
	return m_direction;
}
enum accessPolicy_e 
TSpec::getAccessPolicy (void) const
{
	return m_accessPolicy;
}
uint8_t 
TSpec::getUserPriority (void) const
{
	return m_UP;
}


uint16_t 
TSpec::getNominalMSDUSize (void) const
{
	return (uint16_t)m_nominalMSDUSize;
}
uint16_t 
TSpec::getMaximalMSDUSize (void) const
{
	return (uint16_t)m_maximalMSDUSize;
}
double 
TSpec::getMinimumServiceInterval (void) const
{
	return m_minimumServiceInterval;
}
double 
TSpec::getMaximumServiceInterval (void) const
{
	return m_maximumServiceInterval;
}
double 
TSpec::getInactivityInterval (void) const
{
	return m_inactivityInterval;
}
double 
TSpec::getSuspensionInterval (void) const
{
	return m_suspensionInterval;
}
double 
TSpec::getServiceStartTime (void) const
{
	return m_serviceStartTime;
}
double 
TSpec::getMinimumDataRate (void) const
{
	return m_minimumDataRate;
}
double 
TSpec::getMeanDataRate (void) const
{
	return m_meanDataRate;
}
double 
TSpec::getPeakDataRate (void) const
{
	return m_peakDataRate;
}
uint32_t 
TSpec::getBurstSize (void) const
{
	return m_burstSize;
}
double 
TSpec::getDelayBound (void) const
{
	return m_delayBound;
}
int
TSpec::getMinimumPhyMode (void) const
{
	return m_minimumPhyMode;
}
double 
TSpec::getSurplusBandwidthAllowance (void) const
{
	return m_surplusBandwidthAllowance;
}
double 
TSpec::getMediumTime (void) const
{
	return m_mediumTime;
}











void 
TSpec::setNominalMSDUSize (uint16_t size)
{
	m_nominalMSDUSize = size;
}
void 
TSpec::setMaximalMSDUSize (uint16_t size)
{
	m_maximalMSDUSize = size;
}
void 
TSpec::setMinimumServiceInterval (double interval)
{
	m_minimumServiceInterval = interval;
}
void 
TSpec::setMaximumServiceInterval (double interval)
{
	m_maximumServiceInterval = interval;
}
void 
TSpec::setInactivityInterval (double interval)
{
	m_inactivityInterval = interval;
}
void 
TSpec::setSuspensionInterval (double interval)
{
	m_suspensionInterval = interval;
}
void 
TSpec::setServiceStartTime (double startTime)
{
	m_serviceStartTime = startTime;
}
void 
TSpec::setMinimumDataRate (double rate)
{
	m_minimumDataRate = rate;
}
void 
TSpec::setMeanDataRate (double rate)
{
	m_meanDataRate = rate;
}
void 
TSpec::setPeakDataRate (double rate)
{
	m_peakDataRate = rate;
}
void 
TSpec::setBurstSize (uint32_t size)
{
	m_burstSize = size;
}
void 
TSpec::setDelayBound (double delay)
{
	m_delayBound = delay;
}
void 
TSpec::setMinimumPhyMode (int mode)
{
	m_minimumPhyMode = mode;
}
void 
TSpec::setSurplusBandwidthAllowance (double v)
{
	m_surplusBandwidthAllowance = v;
}
void 
TSpec::setMediumTime (double mediumTime)
{
	m_mediumTime = mediumTime;
}
void
TSpec::setTsid (int tsid)
{
	m_TSID = tsid;
}
