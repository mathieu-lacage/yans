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

#include "sta-rate-control.h"
#include "arf-rate-control.h"
#include "sta-rate-control-factory.h"

#include <assert.h>

class ArfRateControl : public StaRateControl 
{
public:
	ArfRateControl (int min_timer_timeout,
			int min_success_threshold,
			int max_rate,
			int min_rate);
	virtual ~ArfRateControl ();

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
	int m_max_rate;
	int m_min_rate;
	
	int m_rate;
	
	int m_min_timer_timeout;
	int m_min_success_threshold;
	
	char *m_normal_scenario;
	char *m_recovery_scenario;
	
private:
	virtual void reportRecoveryFailure (void);
	virtual void reportFailure (void);
	
protected:
	int getMinTimerTimeout (void);
	int getMinSuccessThreshold (void);
	
	int getTimerTimeout (void);
	int getSuccessThreshold (void);
	
	void setTimerTimeout (int timer_timeout);
	void setSuccessThreshold (int success_threshold);
};

ArfRateControl::ArfRateControl (int min_timer_timeout,
				int min_success_threshold,
				int max_rate,
				int min_rate)
{
        m_min_timer_timeout = min_timer_timeout;
        m_min_success_threshold = min_success_threshold;
        m_max_rate = max_rate;
        m_min_rate = min_rate;
        m_success_threshold = m_min_success_threshold;
        m_timer_timeout = m_min_timer_timeout;
        m_rate = m_min_rate;

        m_success = 0;
        m_failed = 0;
        m_recovery = false;
        m_retry = 0;
        m_timer = 0;

        /* 1/2 */
        m_recovery_scenario = "1000000000000000000000";
        m_normal_scenario   = "01010101010101010101010";
}
ArfRateControl::~ArfRateControl ()
{}

void 
ArfRateControl::reportRTSFailed (void)
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
ArfRateControl::reportDataFailed (void)
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
                        if (m_rate != m_min_rate) {
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
                        if (m_rate != m_min_rate) {
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
ArfRateControl::reportRxOk (double SNR, int mode)
{}
void ArfRateControl::reportRTSOk (double ctsSNR, int ctsMode)
{}
void ArfRateControl::reportDataOk (double ackSNR, int ackMode)
{
	m_timer++;
        m_success++;
        m_failed = 0;
        m_recovery = false;
        m_retry = 0;
        if ((m_success == getSuccessThreshold () ||
             m_timer == getTimerTimeout ()) &&
            (m_rate < (m_max_rate - 1))) {
                m_rate++;
                m_timer = 0;
                m_success = 0;
                m_recovery = true;
        }

}
void ArfRateControl::reportFinalRTSFailed (void)
{}
void ArfRateControl::reportFinalDataFailed (void)
{}
int ArfRateControl::getDataMode (int size)
{
	return m_rate;
}
int ArfRateControl::getRTSMode (void)
{
	return 0;
}

void ArfRateControl::reportRecoveryFailure (void)
{}
void ArfRateControl::reportFailure (void)
{}
int ArfRateControl::getMinTimerTimeout (void)
{
        return m_min_timer_timeout;
}
int ArfRateControl::getMinSuccessThreshold (void)
{
        return m_min_success_threshold;
}
int ArfRateControl::getTimerTimeout (void)
{
        return m_timer_timeout;
}
int ArfRateControl::getSuccessThreshold (void)
{
        return m_success_threshold;
}
void ArfRateControl::setTimerTimeout (int timer_timeout)
{
        assert (timer_timeout >= m_min_timer_timeout);
        m_timer_timeout = timer_timeout;
}
void ArfRateControl::setSuccessThreshold (int success_threshold)
{
        assert (success_threshold >= m_min_success_threshold);
        m_success_threshold = success_threshold;
}




class BroadcastRateControl : public StaRateControl 
{
public:
	BroadcastRateControl ()
	{}
	virtual ~BroadcastRateControl ()
	{}
	virtual void reportRxOk (double SNR, int mode)
	{}
	virtual void reportRTSFailed (void)
	{}
	virtual void reportDataFailed (void) 
	{}
	virtual void reportRTSOk (double ctsSNR, int ctsMode)
	{}
	virtual void reportDataOk (double ackSNR, int ackMode)
	{}
	virtual void reportFinalRTSFailed (void)
	{}
	virtual void reportFinalDataFailed (void)
	{}
	virtual int getDataMode (int size)
	{ return 0; }
	virtual int getRTSMode (void)
	{ return 0;}
};

ArfStaRateControlFactory::ArfStaRateControlFactory ()
{}
ArfStaRateControlFactory::~ArfStaRateControlFactory ()
{}

class StaRateControl *
ArfStaRateControlFactory::createBroadcast (void)
{
	return new BroadcastRateControl ();
}

class StaRateControl *
ArfStaRateControlFactory::create (void)
{
	/* XXX: use mac to access user and PHY params. */
	return new ArfRateControl (15, 10, 0, 5);
}

