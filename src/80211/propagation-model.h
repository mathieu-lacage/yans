/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 */
#ifndef PROPAGATION_MODEL_H
#define PROPAGATION_MODEL_H

#include "callback.tcc"

namespace yans {
/**
 * This propagation model implements a path-loss model
 * with a path loss exponent of 3 and uses 1.0m as its
 * reference distance. This is coherent with the
 * model used by Gavin Holland in "A Rate-Adaptive 
 * MAC Protocol for Multi-Hop Wireless Networks"
 *
 * The tricky aspect of this code relates to the 
 * unit conversions.
 */

class Host;
class Channel80211;
class Packet;

class PropagationData {
public:

	PropagationData (double tx_power, double x, double y, double z);

	double get_x (void) const;
	double get_y (void) const;
	double get_z (void) const;
	double get_tx_power (void) const;
private:
	double m_tx_power;
	double m_x;
	double m_y;
	double m_z;
};

class PropagationModel {
public:
	typedef Callback<void (Packet *, double, int)> RxCallback;
	PropagationModel ();
	~PropagationModel ();

	void set_host (Host *host);
	void set_channel (Channel80211 *channel);
	/* the unit of the power is Watt. */
	void set_receive_callback (RxCallback *callback);

	/* tx power unit: dBm */
	void send (Packet *packet, double tx_power, int tx_mode);
	void receive (Packet *packet, PropagationData const *data, int tx_mode);

	/* unit: dBm */
	void set_tx_gain (double tx_gain);
	/* unit: dBm */
	void set_rx_gain (double rx_gain);
	/* no unit */
	void set_system_loss (double system_loss);
	/* unit: Hz */
	void set_frequency (double frequency);
private:
	double dbm_to_w (double dbm) const;
	double db_to_w (double db) const;
	double get_lambda (void) const;
	double distance (PropagationData const *from) const;
	void forward_up (Packet *packet, double rx_power, int tx_mode);
	double get_rx_power (PropagationData const *rx) const;

	RxCallback *m_rx_callback;
	double m_tx_gain;
	double m_rx_gain;
	double m_system_loss;
	double m_lambda;
	Host *m_host;
	Channel80211 *m_channel;
	static const double PI;
	static const double SPEED_OF_LIGHT;
};

}; // namespace yans

#endif /* PROPAGATION_MODEL_H */
