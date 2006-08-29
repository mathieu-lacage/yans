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

#ifndef TSPEC_H
#define TSPEC_H

#include <stdint.h>

class TSpec {
public:
	TSpec ();

	enum direction_e {
		UPLINK     = 0,
		DOWNLINK   = 2,
		BIDILINK   = 3,
		DIRECTLINK = 1
	};
	enum accessPolicy_e {
		EDCA = 2,
		HCCA = 1,
		HEMM = 3
	};
	enum trafficType_e {
		PERIODIC = 1,
		APERIODIC = 0
	};


	enum trafficType_e getTrafficType (void) const;
	uint8_t getTSID (void) const;
	enum direction_e getLinkDirection (void) const;
	enum accessPolicy_e getAccessPolicy (void) const;
	uint8_t getUserPriority (void) const;

	uint16_t getNominalMSDUSize (void) const;
	uint16_t getMaximalMSDUSize (void) const;
	double getMinimumServiceInterval (void) const;
	double getMaximumServiceInterval (void) const;
	double getInactivityInterval (void) const;
	double getSuspensionInterval (void) const;
	double getServiceStartTime (void) const;
	double getMinimumDataRate (void) const;
	double getMeanDataRate (void) const;
	double getPeakDataRate (void) const;
	uint32_t getBurstSize (void) const;
	double getDelayBound (void) const;
	/* return an index into the PHY transmission 
	 * mode list.
	 */
	int getMinimumPhyMode (void) const;
	double getSurplusBandwidthAllowance (void) const;
	double getMediumTime (void) const;


	void setTrafficType (enum trafficType_e trafficType);
	void setLinkDirection (enum direction_e direction);
	void setAccessPolicy (enum accessPolicy_e policy);
	void setUserPriority (uint8_t up);

	void setTsid (int tsid);
	void setMediumTime (double mediumTime);
	void setMinimumServiceInterval (double interval);
	void setMaximumServiceInterval (double interval);

	void setNominalMsduSize (uint16_t size);
	void setMaximalMsduSize (uint16_t size);
	void setInactivityInterval (double interval);
	void setSuspensionInterval (double interval);
	void setServiceStartTime (double startTime);
	void setMinimumDataRate (double rate);
	void setMeanDataRate (double rate);
	void setPeakDataRate (double rate);
	void setBurstSize (uint32_t size);
	void setDelayBound (double delay);
	void setMinimumPhyMode (int mode);
	void setSurplusBandwidthAllowance (double v);


private:
	// XXX ?
private:
	uint32_t m_nominalMsduSize;
	uint32_t m_maximalMsduSize;
	double m_minimumServiceInterval;
	double m_maximumServiceInterval;
	double m_inactivityInterval;
	double m_suspensionInterval;
	double m_serviceStartTime;
	double m_minimumDataRate;
	double m_meanDataRate;
	double m_peakDataRate;
	uint32_t m_burstSize;
	double m_delayBound;
	uint32_t m_minimumPhyMode;
	double m_surplusBandwidthAllowance;
	double m_mediumTime;

	uint32_t m_TSID        : 4;
	uint32_t m_UP          : 3;
	enum trafficType_e m_trafficType;
	enum direction_e m_direction;
	enum accessPolicy_e m_accessPolicy;

};


#endif /* TSPEC_H */
