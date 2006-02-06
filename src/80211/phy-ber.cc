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

#include <list>
#include <algorithm>
#include <functional>

#include "scheduler.h"
#include "packet.h"

#include "phy-80211.h"
#include "phy-ber.h"
#include "rng-uniform.h"
#include "hdr-mac-80211.h"
#include "transmission-mode.h"

/****************************************************************
 *       Class which records SNIR change events for a 
 *       short period of time.
 ****************************************************************/

PhyBer::NIChange::NIChange (double time, double delta)
	: m_time (time), m_delta (delta) 
{}
double 
PhyBer::NIChange::getTime (void)
{
	return m_time;
}
double 
PhyBer::NIChange::getDelta (void)
{
	return m_delta;
}
bool 
PhyBer::NIChange::operator < (NIChange a) const
{
	return (m_time < a.m_time)?true:false;
}


/****************************************************************
 *       Phy event class
 ****************************************************************/

class PhyRxEvent {
public:
	PhyRxEvent (Packet *packet, PhyBer *phy)
	{
		m_payloadMode = phy->getPayloadMode (packet);
		m_size = ::getSize (packet);
		m_startTime = phy->now ();
		m_endTime = m_startTime + phy->calculatePacketDuration (getHeaderMode (),
									m_payloadMode,
									m_size);
		m_power = phy->calculateRxPower (packet);
		m_refCount = 1;
	}
	~PhyRxEvent ()
	{}
	
	void ref (void)
	{
		m_refCount++;
	}
	void unref (void)
	{
		m_refCount--;
		if (m_refCount == 0) {
			delete this;
		}
	}
	double getDuration (void)
	{
		return m_endTime - m_startTime;
	}
	double getStartTime (void)
	{
		return m_startTime;
	}
	double getEndTime (void)
	{
		return m_endTime;
	}
	bool overlaps (double time)
	{
		if (getStartTime () <= time &&
		    getEndTime () >= time) {
			return true;
		} else {
			return false;
		}
	}
	double getPower (void)
	{
		return m_power;
	}
	int getSize (void) 
	{
		return m_size;
	}
	int getPayloadMode (void) 
	{
		return m_payloadMode;
	}

	int getHeaderMode (void)
	{
		return 0;
	}

private:
	double m_startTime;
	double m_endTime;
	int m_payloadMode;
	int m_size;
	double m_power;
	int m_refCount;
};

/****************************************************************
 *       Rx Handler glue.
 ****************************************************************/

class EndRxHandler : public Handler {
private:
	class RxEvent : public Event 
	{
	public:
		RxEvent (PhyRxEvent *event, Packet *packet)
			: m_event (event),
			  m_packet (packet),
			  m_canceled (false)
		{
			m_event->ref ();
		}
		virtual ~RxEvent ()
		{
			m_event->unref ();
		}
		void cancel (void)
		{
			m_canceled = true;
		}
		bool isCanceled (void)
		{
			return m_canceled;
		}
		class PhyRxEvent *peekPhyRxEvent (void)
		{
			return m_event;
		}
		Packet *peekPacket (void)
		{
			return m_packet;
		}
	private:
		class PhyRxEvent *m_event;
		Packet *m_packet;
		bool m_canceled;
	};
public:
	typedef void (PhyBer::*RxHandler)(class PhyRxEvent *event, Packet *packet);
	EndRxHandler (PhyBer *phy, RxHandler handler)
		: m_phy (phy) , m_handler (handler) ,
		  m_runningEvent (0)
	{}
	virtual ~EndRxHandler ()
	{}
	void start (PhyRxEvent *event, Packet *packet, double delay)
	{
		m_runningEvent = new RxEvent (event, packet);
		Scheduler::instance ().schedule (this, m_runningEvent, delay);
	}
	void cancel (void) 
	{
		m_runningEvent->cancel ();
	}
	virtual void handle (Event *e)
	{
		RxEvent *ev = static_cast<RxEvent *> (e);
		if (!ev->isCanceled ()) {
			(m_phy->*m_handler) (ev->peekPhyRxEvent (), ev->peekPacket ());
		}
		delete ev;
	}
private:
	PhyBer *m_phy;
	RxHandler m_handler;
	RxEvent *m_runningEvent;
};


/****************************************************************
 *       The real BER calculation code.
 ****************************************************************/

RngUniform *PhyBer::m_random = new RngUniform ();

PhyBer::PhyBer ()
	: Phy80211 (),
	  m_endRxHandler (new EndRxHandler (this, &PhyBer::endRx))
{}

PhyBer::~PhyBer ()
{
	delete m_random;
	PhyRxEventsI i = m_rxEventList.begin ();
	while (i != m_rxEventList.end ()) {
		(*i)->unref ();
		i = m_rxEventList.erase (i);
	}
	delete m_endRxHandler;
}

double
PhyBer::calculateNI (PhyRxEvent *phyRxEvent, NIChanges *ni)
{
	PhyRxEventsI i = m_rxEventList.begin ();
	double noiseInterference = 0.0;
	while (i != m_rxEventList.end ()) {
		if (phyRxEvent == (*i)) {
			i++;
			continue;
		}
		if (phyRxEvent->overlaps ((*i)->getStartTime ())) {
			ni->push_back (NIChange ((*i)->getStartTime (), (*i)->getPower ()));
		}
		if (phyRxEvent->overlaps ((*i)->getEndTime ())) {
			ni->push_back (NIChange ((*i)->getEndTime (), -(*i)->getPower ()));
		}
		if ((*i)->overlaps (phyRxEvent->getStartTime ())) {
			noiseInterference += (*i)->getPower ();
		}
		i++;
	}
	ni->push_back (NIChange (phyRxEvent->getStartTime (), noiseInterference));
	ni->push_back (NIChange (phyRxEvent->getEndTime (), 0));

	/* quicksort vector of NI changes by time. */
	std::sort (ni->begin (), ni->end (), std::less<NIChange> ());

	return noiseInterference;
}

double
PhyBer::calculateChunkSuccessRate (double snir, double delay, TransmissionMode *mode)
{
	double rate = mode->getDataRate ();
	double nbits = rate * delay;
	/** XXX: make sure the signed double to unsigned 
	 *  int conversion below is correct.
	 */
	double csr = mode->chunkSuccessRate (snir, ((unsigned int)(int)nbits));
	return csr;
}

double 
PhyBer::calculatePER (PhyRxEvent *packet, NIChanges *ni)
{	
	double psr = 1; /* Packet Success Rate */
	NIChangesI j = ni->begin ();
	double previous = (*j).getTime ();
	double plcpHeaderStart = (*j).getTime () + getPreambleDuration ();
	double plcpPayloadStart = plcpHeaderStart + calculateHeaderDuration (packet->getHeaderMode ());
	double noiseInterference = (*j).getDelta ();
	double power = packet->getPower ();
	TransmissionMode *payloadMode = getMode (packet->getPayloadMode ());
	TransmissionMode *headerMode = getMode (packet->getHeaderMode ());

	j++;
	while (ni->end () != j) {
		assert ((*j).getTime () >= previous);
		
		if (previous >= plcpPayloadStart) {
			psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, payloadMode), 
							  (*j).getTime () - previous,
							  payloadMode);
		} else if (previous >= plcpHeaderStart) {
			if ((*j).getTime () >= plcpPayloadStart) {
				psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, headerMode), 
								  plcpPayloadStart - previous,
								  headerMode);
				psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, payloadMode),
								  (*j).getTime () - plcpPayloadStart,
								  payloadMode);
			} else {
				assert ((*j).getTime () >= plcpHeaderStart);
				psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, headerMode), 
								  (*j).getTime () - previous,
								  headerMode);
			}
		} else {
			if ((*j).getTime () >= plcpPayloadStart) {
				psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, headerMode), 
								  plcpPayloadStart - plcpHeaderStart,
								  headerMode);
				psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, payloadMode), 
								  (*j).getTime () - plcpPayloadStart,
								  payloadMode);
			} else if ((*j).getTime () >= plcpHeaderStart) {
				psr *= calculateChunkSuccessRate (SNR (power, noiseInterference, headerMode), 
								  (*j).getTime () - plcpHeaderStart,
								  headerMode);
			}
		}

		noiseInterference += (*j).getDelta ();
		previous = (*j).getTime ();
		j++;
	}

	double per = 1 - psr;
	return per;
}


double
PhyBer::calculateNoiseInterference (double time)
{
	double noiseInterference;
	list<PhyRxEvent *>::iterator i;

	noiseInterference = 0;
	i = m_rxEventList.begin ();

	while (i != m_rxEventList.end ()) {
		if ((*i)->overlaps (time)) {
			// this packet is contributing to the interference noise
			noiseInterference += (*i)->getPower ();
		}
		i++;
	}

	return noiseInterference;
}



/* signal and noiseInterference are both W
 */

double
PhyBer::calculateCurrentNoiseInterference (void)
{
	double noiseInterference;
	noiseInterference = calculateNoiseInterference (now ());
	return noiseInterference;
}


void 
PhyBer::endRx (PhyRxEvent *phyRxEvent, Packet *packet)
{
	assert (getState () == Phy80211::SYNC);
	assert (phyRxEvent->getEndTime () == 
		Scheduler::instance ().clock ());

	NIChanges ni;

	// XXX: we assign NI to SNR. Wrong !!
	double noiseInterference = calculateNI (phyRxEvent, &ni);
	double snr = SNR (phyRxEvent->getPower (),
			  noiseInterference,
			  getMode (phyRxEvent->getPayloadMode ()));
	setLastRxSNR (snr);

	/* calculate the SNIR at the start of the packet and accumulate
	 * all SNIR changes in the snir vector.
	 */

	double per;
	bool receivedOk;
	per = calculatePER (phyRxEvent, &ni);
	if (m_random->pick () > per) {
		/* success. */
		HDR_CMN (packet)->error () = 0;
		receivedOk = true;
	} else {
		/* failure. */
		HDR_CMN (packet)->error () = 1;
		receivedOk = false;
	}

	HDR_CMN (packet)->direction() = hdr_cmn::UP;
	
	notifyRxEnd (now (), receivedOk);
	switchToIdleFromSync ();
	forwardUp (packet);
}

void 
PhyBer::appendEvent (PhyRxEvent *event)
{
	/* attempt to remove the events which are 
	 * not useful anymore. 
	 * i.e.: all events which end _before_
	 *       now - m_maxPacketDuration
	 */
	double end = now () - getMaxPacketDuration ();
	list<PhyRxEvent *>::iterator i = m_rxEventList.begin ();
	while (i != m_rxEventList.end () &&
	       (*i)->getStartTime () < end) {
		if ((*i)->getEndTime () <= end) {
			(*i)->unref ();
			i = m_rxEventList.erase (i);
		} else {
			i++;
		}
	}
	event->ref ();
	m_rxEventList.push_back (event);
}


void
PhyBer::cancelRx (void)
{
	m_endRxHandler->cancel ();
}

void
PhyBer::startRx (Packet *packet)
{
	PhyRxEvent *event = new PhyRxEvent (packet, this);
	appendEvent (event);

	switch (getState ()) {
	case Phy80211::SYNC:
	case Phy80211::TX:
	case Phy80211::SLEEP:
		Packet::free (packet);
		break;
	case Phy80211::IDLE: {
		double power = calculateRxPower (packet);
		
		if (power > dBmToW (getRxThreshold ())) {
			// sync to signal
			notifyRxStart (now (), event->getDuration ());
			switchToSyncFromIdle (event->getDuration ());
			m_endRxHandler->start (event, packet, 
					       event->getDuration ());
		} else {
			/* if the energy of the signal is smaller than rxThreshold,
			 * this packet is not synced upon.
			 */
			Packet::free (packet);
		}
	} break;
	}
	event->unref ();
}
