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
	virtual void access_granted_now (void) = 0;
	/* ask the listener if there are candidates 
	 * who need access to the medium.
	 */
	virtual bool access_needed (void) = 0;
	/* ask the listener if it is currently
	 * performing an access which was granted 
	 * earlier to him and if it will notify
	 * the Dcf when the access is complete.
	 */
	virtual bool accessing_and_will_notify (void) = 0;
};

class Dcf
{
public:
	Dcf (MacDcfParameters *parameters);
	~Dcf ();

	void set_interface (NetInterface80211 *interface);

	void request_access (void);

	void notify_access_finished (void);
	void notify_access_ongoing_error (void);
	void notify_access_ongoing_error_but_ok (void);
	void notify_access_ongoing_ok (void);

	void register_access_listener (DcfAccessListener *listener);

	MacDcfParameters *parameters (void);

private:
	friend class DcfPhyListener;
	friend class DcfNavListener;

	void access_timeout (MacCancelableEvent *ev);

	/* trivial helpers */
	void reset_cw (void);
	void update_failed_cw (void);
	double pick_backoff_delay (void);
	double now (void) const;
	double most_recent (double a, double b) const;
	double most_recent (double a, double b, double c) const;
	double get_difs (void) const;
	double get_eifs (void) const;

	/* time calculation helpers */
	bool is_backoff_not_completed (double now);
	void start_backoff (void);
	double get_delay_until_access_granted (double now);
	double get_access_granted_start (void) const;
	void update_backoff (double time);

	/* notification methods. */
	void notify_rx_start (double now, double duration);
	void notify_rx_end (double now, bool receivedOk);
	void notify_tx_start (double now, double duration);
	void notify_sleep (double now);
	void notify_wakeup (double now);
	void nav_reset (double now, double duration);
	void nav_start (double now, double duration);
	void nav_continue (double duration);


	NetInterface80211 *m_interface;
	MacDcfParameters   *m_parameters;
	DcfPhyListener *m_phy_listener;
	DcfNavListener *m_nav_listener;
	DynamicHandler<Dcf> *m_access_timer;
	DcfAccessListener *m_listener;

	int m__cw;

	double m_backoff_start;
	double m_backoff_left;
	double m_last_nav_start;
	double m_last_nav_duration;
	double m_last_rx_start;
	double m_last_rx_duration;
	bool m_last_rx_received_ok;
	double m_last_rx_end;
	double m_last_tx_start;
	double m_last_tx_duration;
	double m_last_sleep_start;
	double m_last_wakeup_start;
	bool m_rxing;
	bool m_sleeping;
};

#endif /* DCF_H */
