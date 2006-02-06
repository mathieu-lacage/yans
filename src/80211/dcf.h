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

#ifndef DCF_H
#define DCF_H

#include "mac-handler.tcc"

class DcfPhyListener;
class DcfNavListener;
class MacDcfParameters;
class RngUniform;
class NetInterface80211;
class Phy80211;
class MacLow;

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
	Dcf (MacDcfParameters *parameters);
	~Dcf ();

	void setInterface (NetInterface80211 *interface);

	void requestAccess (void);

	void notifyAccessFinished (void);
	void notifyAccessOngoingError (void);
	void notifyAccessOngoingErrorButOk (void);
	void notifyAccessOngoingOk (void);

	void registerAccessListener (DcfAccessListener *listener);

	MacDcfParameters *parameters (void);

private:
	friend class DcfPhyListener;
	friend class DcfNavListener;

	void accessTimeout (MacCancelableEvent *ev);

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
	void navReset (double now);
	void navStart (double now, double duration);
	void navContinue (double duration);


	NetInterface80211 *m_interface;
	MacDcfParameters   *m_parameters;
	DcfPhyListener *m_phyListener;
	DcfNavListener *m_navListener;
	DynamicHandler<Dcf> *m_accessTimer;
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
