/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2004,2005 INRIA
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

#include "mac-stations.h"
#include "mac-station.h"
#include "arf-mac-stations.h"

#include <assert.h>

class ArfMacStation : public MacStation
{
public:
	ArfMacStation (MacStations *stations,
		       int min_timer_timeout,
		       int min_success_threshold);
	virtual ~ArfMacStation ();

	virtual void reportRxOk (double SNR, int mode);

	virtual void reportRTSFailed (void);
	virtual void reportDataFailed (void);
	virtual void reportRTSOk (double ctsSNR, int ctsMode);
	virtual void reportDataOk (double ackSNR, int ackMode);
	virtual void reportFinalRTSFailed (void);
	virtual void reportFinalDataFailed (void);
	virtual int getDataMode (int size);
	virtual int getRTSMode (void);

private:
	int m_timer;
	int m_success;
	int m_failed;
	bool m_recovery;
	int m_retry;
	
	int m_timer_timeout;
	int m_success_threshold;

	int m_rate;
	
	int m_min_timer_timeout;
	int m_min_success_threshold;
	
	char *m_normal_scenario;
	char *m_recovery_scenario;
	
private:
	virtual void reportRecoveryFailure (void);
	virtual void reportFailure (void);

	int getMaxRate (void);
	int getMinRate (void);
	
protected:
	int getMinTimerTimeout (void);
	int getMinSuccessThreshold (void);
	
	int getTimerTimeout (void);
	int getSuccessThreshold (void);
	
	void setTimerTimeout (int timer_timeout);
	void setSuccessThreshold (int success_threshold);
};

ArfMacStation::ArfMacStation (MacStations *stations,
			      int min_timer_timeout,
			      int min_success_threshold)
	: MacStation (stations)
{
        m_min_timer_timeout = min_timer_timeout;
        m_min_success_threshold = min_success_threshold;
        m_success_threshold = m_min_success_threshold;
        m_timer_timeout = m_min_timer_timeout;
        m_rate = getMinRate ();

        m_success = 0;
        m_failed = 0;
        m_recovery = false;
        m_retry = 0;
        m_timer = 0;

        /* 1/2 */
        m_recovery_scenario = "1000000000000000000000";
        m_normal_scenario   = "01010101010101010101010";
}
ArfMacStation::~ArfMacStation ()
{}

int 
ArfMacStation::getMaxRate (void)
{
	return getNModes ();
}
int 
ArfMacStation::getMinRate (void)
{
	return 0;
}


void 
ArfMacStation::reportRTSFailed (void)
{}
/**
 * It is important to realize that "recovery" mode starts after failure of
 * the first transmission after a rate increase and ends at the first successful
 * transmission. Specifically, recovery mode transcends retransmissions boundaries.
 * Fundamentally, ARF handles each data transmission independently, whether it
 * is the initial transmission of a packet or the retransmission of a packet.
 * The fundamental reason for this is that there is a backoff between each data
 * transmission, be it an initial transmission or a retransmission.
 */
void 
ArfMacStation::reportDataFailed (void)
{
        m_timer++;
        m_failed++;
        m_retry++;
        m_success = 0;

        if (m_recovery) {
                assert (m_retry >= 1);
                assert (m_retry - 1 <= strlen (m_recovery_scenario));
                if (m_recovery_scenario[m_retry-1] == '1') {
                        reportRecoveryFailure ();
                        if (m_rate != getMinRate ()) {
                                m_rate--;
                        }
                } else if (m_recovery_scenario[m_retry-1] == '0') {
                }
                m_timer = 0;
        } else {
                assert (m_retry >= 1);
                assert (m_retry - 1 <= strlen (m_normal_scenario));
                if (m_normal_scenario[m_retry-1] == '1') {
                        reportFailure ();
                        if (m_rate != getMinRate ()) {
                                m_rate--;
                        }
                } else if (m_normal_scenario[m_retry-1] == '0') {
                }
                if (m_retry >= 2) {
                        m_timer = 0;
                }
        }
}
void 
ArfMacStation::reportRxOk (double SNR, int mode)
{}
void ArfMacStation::reportRTSOk (double ctsSNR, int ctsMode)
{}
void ArfMacStation::reportDataOk (double ackSNR, int ackMode)
{
	m_timer++;
        m_success++;
        m_failed = 0;
        m_recovery = false;
        m_retry = 0;
        if ((m_success == getSuccessThreshold () ||
             m_timer == getTimerTimeout ()) &&
            (m_rate < (getMaxRate () - 1))) {
                m_rate++;
                m_timer = 0;
                m_success = 0;
                m_recovery = true;
        }

}
void ArfMacStation::reportFinalRTSFailed (void)
{}
void ArfMacStation::reportFinalDataFailed (void)
{}
int ArfMacStation::getDataMode (int size)
{
	return m_rate;
}
int ArfMacStation::getRTSMode (void)
{
	return 0;
}

void ArfMacStation::reportRecoveryFailure (void)
{}
void ArfMacStation::reportFailure (void)
{}
int ArfMacStation::getMinTimerTimeout (void)
{
        return m_min_timer_timeout;
}
int ArfMacStation::getMinSuccessThreshold (void)
{
        return m_min_success_threshold;
}
int ArfMacStation::getTimerTimeout (void)
{
        return m_timer_timeout;
}
int ArfMacStation::getSuccessThreshold (void)
{
        return m_success_threshold;
}
void ArfMacStation::setTimerTimeout (int timer_timeout)
{
        assert (timer_timeout >= m_min_timer_timeout);
        m_timer_timeout = timer_timeout;
}
void ArfMacStation::setSuccessThreshold (int success_threshold)
{
        assert (success_threshold >= m_min_success_threshold);
        m_success_threshold = success_threshold;
}





ArfMacStations::ArfMacStations (MacContainer *container)
	: MacStations (container)
{}
ArfMacStations::~ArfMacStations ()
{}
MacStation *
ArfMacStations::createStation (void)
{
	/* XXX: use mac to access user and PHY params. */
	return new ArfMacStation (this, 15, 10);
}

