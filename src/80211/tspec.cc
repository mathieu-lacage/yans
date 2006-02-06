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

#include "tspec.h"

TSpec::TSpec ()
	: m_nominalMsduSize (1500),
	  m_maximalMsduSize (1500),
	  m_minimumServiceInterval (0.0),
	  m_maximumServiceInterval (1.0),
	  m_inactivityInterval (1.0),
	  m_suspensionInterval (1.0),
	  m_serviceStartTime (1.0),
	  m_minimumDataRate (100000),
	  m_meanDataRate (10000),
	  m_peakDataRate (10000),
	  m_burstSize (4000),
	  m_delayBound (0.0),
	  m_minimumPhyMode (0),
	  m_surplusBandwidthAllowance (1.5),
	  m_mediumTime (1.0)
{}

enum TSpec::trafficType_e
TSpec::getTrafficType (void) const
{
	return m_trafficType;
}
uint8_t 
TSpec::getTSID (void) const
{
	return m_TSID;
}
enum TSpec::direction_e 
TSpec::getLinkDirection (void) const
{
	return m_direction;
}
enum TSpec::accessPolicy_e 
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
	return (uint16_t)m_nominalMsduSize;
}
uint16_t 
TSpec::getMaximalMSDUSize (void) const
{
	return (uint16_t)m_maximalMsduSize;
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
TSpec::setTrafficType (enum trafficType_e trafficType)
{
	m_trafficType = trafficType;
}
void 
TSpec::setLinkDirection (enum direction_e direction)
{
	m_direction = direction;
}
void 
TSpec::setAccessPolicy (enum accessPolicy_e policy)
{
	m_accessPolicy = policy;
}
void 
TSpec::setUserPriority (uint8_t up)
{
	m_UP = up;
}

void 
TSpec::setNominalMsduSize (uint16_t size)
{
	m_nominalMsduSize = size;
}
void 
TSpec::setMaximalMsduSize (uint16_t size)
{
	m_maximalMsduSize = size;
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
