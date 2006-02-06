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
#ifndef RATE_CONTROL_H
#define RATE_CONTROL_H

#include <map>

class RateControl {
public:
	RateControl (class StaRateControlFactory *m_factory);
	~RateControl ();
	void reportRxOk (int source, double SNR, int mode);

	void reportRTSFailed (int destination);
	void reportDataFailed (int destination);
	void reportRTSOk (int destination, double ctsSNR, int ctsMode);
	void reportDataOk (int destination, double ackSNR, int ackMode);

	void reportFinalRTSFailed (int destination);
	void reportFinalDataFailed (int destination);

	int getDataMode (int destination, int size);
	int getRTSMode (int destination);
	int getBroadcastDataMode (int size);
private:
	class StaRateControl *peekRateControl (int destination);

	class StaRateControlFactory *m_factory;
	class StaRateControl *m_broadcast;
	std::map <int, class StaRateControl *, std::less<int> > m_destinations;
};


#endif /* RATE_CONTROL_H */
