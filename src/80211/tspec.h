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

#ifndef TSPEC_H
#define TSPEC_H

#include <stdint.h>
#include <tclcl.h>

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

class TSpec : public TclObject {
public:
	TSpec ();

	enum trafficType_e getTrafficType (void);
	uint8_t getTSID (void);
	enum direction_e getLinkDirection (void);
	enum accessPolicy_e getAccessPolicy (void);
	uint8_t getUserPriority (void);	

	uint16_t getNominalMSDUSize (void);
	uint16_t getMaximalMSDUSize (void);
	double getMinimumServiceInterval (void);
	double getMaximumServiceInterval (void);
	double getInactivityInterval (void);
	double getSuspensionInterval (void);
	double getServiceStartTime (void);
	double getMinimumDataRate (void);
	double getMeanDataRate (void);
	double getPeakDataRate (void);
	uint32_t getBurstSize (void);
	double getDelayBound (void);
	double getMinimumPhyRate (void);
	double getSurplusBandwidthAllowance (void);
	double getMediumTime (void);

private:
	// XXX ?
	void setNominalMSDUSize (uint16_t size);
	void setMaximalMSDUSize (uint16_t size);
	void setMinimumServiceInterval (double interval);
	void setMaximumServiceInterval (double interval);
	void setInactivityInterval (double interval);
	void setSuspensionInterval (double interval);
	void setServiceStartTime (double startTime);
	void setMinimumDataRate (double rate);
	void setMeanDataRate (double rate);
	void setPeakDataRate (double rate);
	void setBurstSize (uint32_t size);
	void setDelayBound (double delay);
	void setMinimumPhyRate (double rate);
	void setSurplusBandwidthAllowance (double v);
	void setMediumTime (double mediumTime);
private:
	uint32_t m_nominalMSDUSize;
	uint32_t m_maximalMSDUSize;
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
	double m_minimumPhyRate;
	double m_surplusBandwidthAllowance;
	double m_mediumTime;

	uint32_t m_TSID        : 4;
	uint32_t m_UP          : 3;
	enum trafficType_e m_trafficType;
	enum direction_e m_direction;
	enum accessPolicy_e m_accessPolicy;


	void initialize (char const *varName, uint32_t *variable, uint32_t defaultValue);
	void initialize (char const *varName, uint16_t *variable, uint16_t defaultValue);
	void initialize (char const *varName, double *variable, double defaultValue);
	bool isDefined (char const *varName);

	virtual int command(int argc, const char*const* argv);

};


#endif /* TSPEC_H */
