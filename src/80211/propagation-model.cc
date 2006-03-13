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
 * Authors: Hossein Manshaei, Mathieu Lacage
 */
#include "propagation-model.h"
#include "host.h"
#include "channel-80211.h"
#include "simulator.h"
#include "packet.h"
#include "count-ptr-holder.tcc"
#include "event.tcc"
#include <math.h>

#define PROP_DEBUG 1

#ifdef PROP_DEBUG
#include <iostream>
#  define TRACE(x) \
std::cout << "PROP TRACE " << Simulator::now_s () << " " << x << std::endl;
#else
#  define TRACE(x)
#endif


namespace yans {

const double PropagationModel::PI = 3.1415;
const double PropagationModel::SPEED_OF_LIGHT = 300000000;

PropagationData::PropagationData (double tx_power_dbm, double x, double y, double z)
	: m_tx_power_dbm (tx_power_dbm), m_x (x), m_y (y), m_z (z)
{}

double 
PropagationData::get_x (void) const
{
	return m_x;
}
double 
PropagationData::get_y (void) const
{
	return m_y;
}
double 
PropagationData::get_z (void) const
{
	return m_z;
}
double 
PropagationData::get_tx_power_dbm (void) const
{
	return m_tx_power_dbm;
}


PropagationModel::PropagationModel ()
	: m_rx_callback (0)
{}
PropagationModel::~PropagationModel ()
{
	delete m_rx_callback;
}

void 
PropagationModel::set_host (Host *host)
{
	m_host = host;
}
	
void 
PropagationModel::set_channel (Channel80211 *channel)
{
	m_channel = channel;
}
void 
PropagationModel::set_receive_callback (RxCallback *callback)
{
	m_rx_callback = callback;
}

void 
PropagationModel::send (Packet *packet, double tx_power_dbm, 
			uint8_t tx_mode, uint8_t stuff) const
{
	PropagationData data (tx_power_dbm + m_tx_gain_dbm, m_host->get_x (),
			      m_host->get_y (), m_host->get_z ());
	m_channel->send (packet, &data, tx_mode, stuff, this);
}
void 
PropagationModel::receive (Packet const*packet, PropagationData const *data, 
			   uint8_t tx_mode, uint8_t stuff)
{
	double rx_power = get_rx_power (data);
	double delay = distance (data) / 300000000;
	Simulator::insert_in_s (delay, make_event (&PropagationModel::forward_up, 
						   this, make_count_ptr_holder (packet),
						   rx_power, tx_mode, stuff));
}

void
PropagationModel::forward_up (CountPtrHolder<Packet const> p, double rx_power, uint8_t tx_mode, uint8_t stuff)
{
	Packet const*packet = p.remove ();
	(*m_rx_callback) (packet, rx_power, tx_mode, stuff);
	packet->unref ();
}

double
PropagationModel::distance (PropagationData const *from) const
{
	double dx = m_host->get_x () - from->get_x ();
	double dy = m_host->get_y () - from->get_y ();
	double dz = m_host->get_z () - from->get_z ();
	return sqrt (dx*dx+dy*dy+dz*dz);
}

double 
PropagationModel::get_rx_power (PropagationData const *rx) const
{
	double dist = distance (rx);
	if (dist <= 1.0) {
		// XXX
		return dbm_to_w (rx->get_tx_power_dbm () + m_rx_gain_dbm);
	}
	/*
	 * Friis free space equation:
	 * where Pt, Gr, Gr and P are in Watt units
	 * L is in meter units.
	 *
	 *       Pt * Gt * Gr * (lambda^2)
	 *   P = --------------------------
	 *       (4 * pi * d)^2 * L
	 *
	 * But, here, we have the following 
	 * variables/units:
	 * 
	 * L = m_system_loss
	 * Gt = m_tx_gain (dBm)
	 * Gr = m_rx_gain (dBm)
	 * Pt = tx_power (dBm)
	 * d = 1.0m
	 */
	double numerator = dbm_to_w (rx->get_tx_power_dbm () + m_rx_gain_dbm) * m_lambda * m_lambda;
	double denominator = 16 * PI * PI * 1.0 * 1.0 * m_system_loss;
	double prd0 = numerator / denominator;
	
	double n = 3.0; /* path loss exponent */
	double pr = 10*log10(prd0) - n * 10.0 * log10(dist);
	return db_to_w (pr);
}

void 
PropagationModel::set_tx_gain_dbm (double tx_gain)
{
	m_tx_gain_dbm = tx_gain;
}
void 
PropagationModel::set_rx_gain_dbm (double rx_gain)
{
	m_rx_gain_dbm = rx_gain;
}
void 
PropagationModel::set_system_loss (double system_loss)
{
	m_system_loss = system_loss;
}
void 
PropagationModel::set_frequency_hz (double frequency)
{
	const double speed_of_light = 300000000;
	double lambda = speed_of_light / frequency;
	m_lambda = lambda;
}
double 
PropagationModel::dbm_to_w (double dbm) const
{
	double mw = pow(10.0,dbm/10.0);
	return mw / 1000.0;
}
double
PropagationModel::db_to_w (double db) const
{
	return pow(10.0,db/10.0);
}


}; // namespace yans
