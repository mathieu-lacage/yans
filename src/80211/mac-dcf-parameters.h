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
#ifndef MAC_DCF_PARAMETERS_H
#define MAC_DCF_PARAMETERS_H

#include <stdint.h>

#include "hdr-mac-80211.h"

class MacParameters;
class Phy80211;

class MacDcfParameters {
public:
	MacDcfParameters (void);

	void setParameters (MacParameters *parameters);

	int getCWmin (void);
	int getCWmax (void);
	double getTxopLimit (void);
	double getAIFS (void);
	double getEIFS (Phy80211 *phy);

	bool isAccessAllowed (void);
	void startAccessAllowed (double delay);

	bool isACMandatory (void);

	void setCWmin (uint16_t CWmin);
	void setCWmax (uint16_t CWmin);
	void setTxopLimit (double txopLimit);
	void setAIFSN (uint8_t AIFSN);
	void setACM (bool enabled);



	void writeTo (uint8_t buffer[4], enum ac_e ac);
	void readFrom (uint8_t const buffer[4]);
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
