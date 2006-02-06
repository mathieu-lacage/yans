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

#ifndef BACKOFF_H
#define BACKOFF_H

#include "phy-80211.h"

class MacLow;

class Backoff : public Phy80211Listener
{
 public:
	Backoff (MacLow *mac);
	~Backoff ();
	void start (double backoffStart, double backoffDuration);
	void cancel (void);
	bool isCompleted (double now);
	double getDelayUntilEnd (double now);
	bool isNavZero (double now) const;
	double getDelayUntilNavZero (double now) const;
	/* Return how many seconds we believe we have left until the
	 * backoff expires. If the backoff has already expired, 
	 * this method returns zero.
	 */
	double getDelayUntilAccessAllowed (double now) const;

	bool wasLastRxOk (void) const;
	double getLastRxEndTime (void) const;
	double getLastTxEndTime (void) const;

	void notifyNav (double start, double duration);
	virtual void notifyRxStart (double now, double duration);
	virtual void notifyRxEnd (double now, bool receivedOk);
	virtual void notifyTxStart (double now, double duration);
	virtual void notifySleep (double now);
	virtual void notifyWakeup (double now);

private:
	double max (double a, double b) const;
	double mostRecent (double a, double b, double c) const;
	void updateBackoff (double time);
	double getDIFS (void) const;
	double getEIFS (void) const;
	double getAccessAllowedStart (void) const;

	MacLow *m_mac;
	double m_backoffStart;
	double m_backoffLeft;
	double m_lastNavStart;
	double m_lastNavDuration;
	double m_lastRxStart;
	double m_lastRxDuration;
	bool m_lastRxReceivedOk;
	double m_lastRxEnd;
	double m_lastTxStart;
	double m_lastTxDuration;
	double m_lastSleepStart;
	double m_lastWakeupStart;
	bool m_rxing;
	bool m_sleeping;
};

#endif /* BACKOFF_H */
