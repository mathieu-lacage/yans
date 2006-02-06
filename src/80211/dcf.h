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

#ifndef DCF_H
#define DCF_H

#include "mac-handler.tcc"

class MacContainer;
class DcfPhyListener;
class DcfNavListener;
class MacDcfParameters;
class RngUniform;

class DcfAccessListener {
public:
	DcfAccessListener ();
	virtual ~DcfAccessListener ();

	/* Tell the listener than it can start
	 * accessing the medium right now.
	 */
	virtual void accessGrantedNow (void) = 0;
	/* ask the listener if there are candidates 
	 * who need access to the medium.
	 */
	virtual bool accessNeeded (void) = 0;
	/* ask the listener if it is currently
	 * performing an access which was granted 
	 * earlier to him and if it will notify
	 * the Dcf when the access is complete.
	 */
	virtual bool accessingAndWillNotify (void) = 0;
};

class Dcf
{
 public:
	Dcf (MacContainer *container, MacDcfParameters *parameters);
	~Dcf ();

	void requestAccess (void);
	void notifyAccessOk (void);
	void notifyAccessFailed (void);
	void registerAccessListener (DcfAccessListener *listener);

private:
	friend class DcfPhyListener;
	friend class DcfNavListener;

	void accessTimeout (void);

	/* trivial helpers */
	void resetCW (void);
	void updateFailedCW (void);
	double pickBackoffDelay (void);
	double now (void) const;
	double mostRecent (double a, double b) const;
	double mostRecent (double a, double b, double c) const;
	double getDIFS (void) const;
	double getEIFS (void) const;

	/* time calculation helpers */
	bool isBackoffNotCompleted (double now);
	void startBackoff (void);
	double getDelayUntilAccessGranted (double now);
	double getAccessGrantedStart (void) const;
	void updateBackoff (double time);

	/* notification methods. */
	void notifyRxStart (double now, double duration);
	void notifyRxEnd (double now, bool receivedOk);
	void notifyTxStart (double now, double duration);
	void notifySleep (double now);
	void notifyWakeup (double now);
	void navStart (double now, double duration);
	void navContinue (double duration);


	MacContainer *m_container;
	MacDcfParameters   *m_parameters;
	DcfPhyListener *m_phyListener;
	DcfNavListener *m_navListener;
	StaticHandler<Dcf> *m_accessTimer;
	DcfAccessListener *m_listener;

	int m_CW;

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

#endif /* DCF_H */
