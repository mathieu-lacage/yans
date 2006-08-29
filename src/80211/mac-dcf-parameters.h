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
#ifndef MAC_DCF_PARAMETERS_H
#define MAC_DCF_PARAMETERS_H

#include "yans/stdint.h"

#include "hdr-mac-80211.h"

class MacParameters;
class Phy80211;

class MacDcfParameters {
public:
	MacDcfParameters (void);

	void set_parameters (MacParameters *parameters);

	int get_cwmin (void);
	int get_cwmax (void);
	double get_txop_limit (void);
	double get_aifs (void);
	double get_eifs (Phy80211 *phy);

	bool is_access_allowed (void);
	void start_access_allowed (double delay);

	bool is_acmandatory (void);

	void set_cwmin (uint16_t CWmin);
	void set_cwmax (uint16_t CWmin);
	void set_txop_limit (double txopLimit);
	void set_aifsn (uint8_t AIFSN);
	void set_acm (bool enabled);



	void write_to (uint8_t buffer[4], enum ac_e ac);
	void read_from (uint8_t const buffer[4]);
private:
	uint8_t log2 (uint16_t c);

	MacParameters *m_parameters;
	double m_txopLimit;
	uint16_t m_CWmin;
	uint16_t m_CWmax;
	bool m_ACM;
	uint8_t m_AIFSN;
};

#endif /* MAC_DCF_PARAMETERS_H */
