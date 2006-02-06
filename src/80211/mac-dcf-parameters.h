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

#include <stdint.h>

class MacContainer;

class MacDcfParameters {
public:
	MacDcfParameters (MacContainer *container);
	int getCWmin (void);
	int getCWmax (void);
	double getTxopLimit (void);
	double getAIFS (void);
	double getEIFS (void);

	bool isAccessAllowed (void);
	void startAccessAllowed (double delay);

	bool isACMandatory (void);

	void setCWmin (uint16_t CWmin);
	void setCWmax (uint16_t CWmin);
	void setTxopLimit (double txopLimit);
	void setAIFSN (uint8_t AIFSN);
	void setACM (uint8_t ACM);
private:
	MacContainer *m_container;

	double m_txopLimit;
	uint16_t m_CWmin;
	uint16_t m_CWmax;
	bool m_ACM;
	uint8_t m_AIFSN;
};

#endif /* MAC_DCF_PARAMETERS_H */
