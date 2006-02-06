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

#ifndef PHY_EVENT_LIST_H
#define PHY_EVENT_LIST_H

#include <list>

class PhyStateEvent {
public:
	enum PhyStateEventType {
		TX_START,
		TX_END,
		SYNC_START,
		SYNC_END,
		SLEEP,
		WAKEUP
	};

	PhyStateEvent (enum PhyStateEventType type, double time);

	enum PhyStateEventType getType (void);
	double getTime (void);
private:
	enum PhyStateEventType m_type;
	double m_time;
};

class PhyStateEventSyncEnd : public PhyStateEvent {
public:
	PhyStateEventSyncEnd (double time, bool wasSuccessful) ;
	bool getWasSuccessful (void);
private:
	bool m_wasSuccessful;
};


class Phy80211EventList {
public:
	Phy80211EventList ();
	virtual ~Phy80211EventList ();

	void notifyRxStart (double now);
	void notifyRxEnd (double now, bool receivedOk);
	void notifyTxStart (double now, double end);
	void notifySleep (double now);
	void notifyWakeup (double now);

	std::list<PhyStateEvent *>::const_iterator begin (void);
	std::list<PhyStateEvent *>::const_iterator end (void);
	int size (void);

	double getLastRxEndTime (void);
	bool wasLastRxEndSuccessful (void);
	double getLastTxEndTime (void);

	int  notifyBackoffStart (double now);
	void notifyBackoffEnd (int backoffId);
	void notifyBackoffHandledUntilNow (int backoffId, double now);
private:
	void clearEventList (void);
	void clearEventListUntil (double until);
	double min (double a, double b);
	
	std::list<PhyStateEvent *> m_list;
	std::list<std::pair<int, double> >m_backoffReservations;
	int m_backoffReservation;
	double m_lastRxEndTime;
	bool m_wasLastRxSuccessful;
	double m_lastTxEndTime;
};

#endif /* PHY_EVENT_LIST_H */
