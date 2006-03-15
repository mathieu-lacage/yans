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

#include <stdint.h>
#include "cancellable-event.h"

namespace yans {

class RandomUniform;
class MacParameters;

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
	Dcf ();
	~Dcf ();

	void set_parameters (MacParameters const*parameters);
	void set_difs_us (uint64_t difs_us);
	void set_eifs_us (uint64_t eifs_us);
	void set_cw_bounds (uint32_t min, uint32_t max);
	void register_access_listener (DcfAccessListener *listener);

	void request_access (bool is_phy_busy);

	void notify_access_finished (void);
	void notify_access_ongoing_error (void);
	void notify_access_ongoing_error_but_ok (void);
	void notify_access_ongoing_ok (void);

	/* notification methods. */
	void notify_rx_start (uint64_t now, uint64_t duration);
	void notify_rx_end (uint64_t now, bool receivedOk);
	void notify_tx_start (uint64_t now, uint64_t duration);
	void notify_sleep (uint64_t now);
	void notify_wakeup (uint64_t now);
	void nav_reset (uint64_t now, uint64_t duration);
	void nav_start (uint64_t now, uint64_t duration);
	void nav_continue (uint64_t duration);
private:
	void access_timeout (void);

	/* trivial helpers */
	void reset_cw (void);
	void update_failed_cw (void);
	uint64_t pick_backoff_delay (void);
	uint64_t now_us (void) const;
	uint64_t most_recent (uint64_t a, uint64_t b) const;
	uint64_t most_recent (uint64_t a, uint64_t b, uint64_t c) const;
	uint64_t get_difs_us (void) const;
	uint64_t get_eifs_us (void) const;
	uint32_t get_cw_min (void) const;
	uint32_t get_cw_max (void) const;

	/* time calculation helpers */
	bool is_backoff_not_completed (uint64_t now);
	void start_backoff (void);
	uint64_t get_delay_until_access_granted (uint64_t now);
	uint64_t get_access_granted_start (void) const;
	void update_backoff (uint64_t time_us);

	CancellableEvent *m_access_timer_event;

	RandomUniform *m_random;
	MacParameters const*m_parameters;
	DcfAccessListener *m_listener;
	uint64_t m_difs_us;
	uint64_t m_eifs_us;
	uint32_t m_cw_min;
	uint32_t m_cw_max;

	uint32_t m_cw;
	uint64_t m_backoff_start;
	uint64_t m_backoff_left;
	uint64_t m_last_nav_start;
	uint64_t m_last_nav_duration;
	uint64_t m_last_rx_start;
	uint64_t m_last_rx_duration;
	bool m_last_rx_received_ok;
	uint64_t m_last_rx_end;
	uint64_t m_last_tx_start;
	uint64_t m_last_tx_duration;
	uint64_t m_last_sleep_start;
	uint64_t m_last_wakeup_start;
	bool m_rxing;
	bool m_sleeping;
};

}; // namespace yans

#endif /* DCF_H */
