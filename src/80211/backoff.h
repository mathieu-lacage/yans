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

class MacLow;
class Phy80211;
class BackoffPhyListener;
class BackoffNavListener;
class MacLowParameters;

class DcfAccessListener {
public:
	DcfAccessListener ();

	virtual void accessGrantedNow (void);
};


class Dcf
{
 public:
	Backoff (Phy80211 *phy, MacLow *mac, MacLowParameters *parameters);
	~Backoff ();

	void requestAccess (void);
	void notifyAccessOk (void);
	void notifyAccessFailed (void);
	void registerAcessListener (DcfAccessListener *listener);

private:
	friend class BackoffPhyListener;
	friend class BackoffNavListener;

	void accessTimeout (void);

	/* trivial helpers */
	bool wasLastRxOk (void) const;
	double getLastRxEndTime (void) const;
	double getLastTxEndTime (void) const;
	double mostRecent (double a, double b) const;
	double mostRecent (double a, double b, double c) const;
	double getDIFS (void) const;
	double getEIFS (void) const;

	/* time calculation helpers */
	void recordBackoffstarted (double backoffStart, double backoffDuration);
	double getDelayUntilAccessAllowed (double now) const;
	double getAccessAllowedStart (void) const;
	void updateBackoff (double time);

	/* notification methods. */
	void notifyRxStart (double now, double duration);
	void notifyRxEnd (double now, bool receivedOk);
	void notifyTxStart (double now, double duration);
	void notifySleep (double now);
	void notifyWakeup (double now);
	void notifyNav (double now, double duration);


	MacLowParameters   *m_parameters;
	BackoffPhyListener *m_phyListener;
	BackoffNavListener *m_navListener;

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
