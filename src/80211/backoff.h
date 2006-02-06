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

#include <list>

class NavEvent;
class Phy80211EventList;
class MacLow80211;

class Backoff 
{
 public:
	Backoff (MacLow80211 *mac);
	/* start a backoff from now */
	void start (double backoffStart, double backoffDuration);
	void cancel (void);
	/* a packet has been received and its contained
	 * a valid duration field which might require a
	 * NAV update.
	 */
	void updateNAV (double duration, double now);
	/* is the current backoff completed. This method
	 * requires a call to updateToNow before.
	 */
	bool isCompleted (void) const;
	/* how long we have left in the current backoff. 
	 * This method
	 * requires a call to updateToNow before.
	 */
	double getDurationLeft (void) const;

	/* This operation can be really slow. */
	void updateToNow (double now);

	bool isVirtualCS_Idle (double now);
	double getDelayUntilIdle (double now);

	/* Return how many seconds we believe we have left until the
	 * backoff expires. If the backoff has already expired, 
	 * this method returns zero.
	 */
	double getExpectedDelayToEndFromNow (double now);

private:
	Phy80211EventList *getPhyEventList (void);
	double max (double a, double b);
	double mostRecent (double a, double b, double c, double d);
	void updateBackoff (double time);
	void recordTxStart (double time);
	void recordTxEnd (double time);
	void recordSyncStart (double time);
	void recordSyncEnd (double time, bool receivedOk);
	void recordSleep (double time);
	void recordWakeup (double time);
	void recordNAV_BusyStart (double time);
	void recordNAV_BusyEnd (double time);
	void clearNAV_Until (double time);
	double getDIFS (void);
	double getEIFS (void);

	MacLow80211 *m_mac;
	std::list<NavEvent *> m_navList;
	double m_backoffStart;
	double m_backoffLeft;
	bool   m_NAV_Running;
	double m_lastNAV_BackoffStart;
	double m_lastTxBackoffStart;
	double m_lastRxBackoffStart;
	int m_backoffId;
	bool m_txing;
	bool m_rxing;
	bool m_sleeping;
};

#endif /* BACKOFF_H */
