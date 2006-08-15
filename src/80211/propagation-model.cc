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
#include "position.h"
#include "channel-80211.h"
#include "simulator.h"
#include "gpacket.h"
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



PropagationModel::PropagationModel ()
{}
PropagationModel::~PropagationModel ()
{}

void 
PropagationModel::set_position (Position *position)
{
	m_position = position;
}
	
void 
PropagationModel::set_channel (BaseChannel80211 *channel)
{
	m_channel = channel;
}
void 
PropagationModel::set_receive_callback (RxCallback callback)
{
	m_rx_callback = callback;
}

void 
PropagationModel::send (Packet const packet, double tx_power_dbm, 
			uint8_t tx_mode, uint8_t stuff) const
{
	m_channel->send (packet, tx_power_dbm + m_tx_gain_dbm, 
			 tx_mode, stuff, this);
}
void
PropagationModel::get_position (double &x, double &y, double &z) const
{
	m_position->get (x, y, z);
}
uint64_t
PropagationModel::get_prop_delay_us (double from_x, double from_y, double from_z) const
{
	double dist = distance (from_x, from_y, from_z);
	uint64_t delay_us = (uint64_t) (dist / 300000000 * 1000000);
	return delay_us;
}
double
PropagationModel::get_rx_power_w (double tx_power_dbm, double from_x, double from_y, double from_z) const
{
	double dist = distance (from_x, from_y, from_z);
	double rx_power_w = get_rx_power_w (tx_power_dbm, dist);
	return rx_power_w;
}
void 
PropagationModel::receive (Packet const packet, 
			   double rx_power_w,
			   uint8_t tx_mode, uint8_t stuff)
{
	m_rx_callback (packet, rx_power_w, tx_mode, stuff);
}

double
PropagationModel::distance (double from_x, double from_y, double from_z) const
{
	double x,y,z;
	m_position->get (x,y,z);
	double dx = x - from_x;
	double dy = y - from_y;
	double dz = z - from_z;
	return sqrt (dx*dx+dy*dy+dz*dz);
}

double 
PropagationModel::get_rx_power_w (double tx_power_dbm, double dist) const
{
	if (dist <= 1.0) {
		// XXX
		return dbm_to_w (tx_power_dbm + m_rx_gain_dbm);
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
	double numerator = dbm_to_w (tx_power_dbm + m_rx_gain_dbm) * m_lambda * m_lambda;
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
