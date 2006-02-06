/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
 *
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifdef DESTINATION_RATE_CONTROL_H
#define DESTINATION_RATE_CONTROL_H

class DestinationRateControl {
public:
	DestinationRateControl ();
	virtual ~DestinationRateControl () = 0;

	virtual void reportRxOk (double SNR, int mode) = 0;

	virtual void reportRTSFailed (void) = 0;
	virtual void reportDataFailed (void) = 0;
	virtual void reportRTSOk (double ctsSNR, int ctsMode) = 0;
	virtual void reportDataOk (double ackSNR, int ackMode) = 0;
	virtual void reportFinalRTSFailed (void) = 0;
	virtual void reportFinalDataFailed (void) = 0;
	virtual int getDataMode (int size) = 0;
	virtual int getRTSMode (void) = 0;
};

#endif /* DESTINATION_RATE_CONTROL_H */
