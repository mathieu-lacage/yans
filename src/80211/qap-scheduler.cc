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

#include <math.h>
#include <algorithm>

/* This scheduler must:
 *  - perform admission control
 *  - generate properly-timed beacons
 *  - generate properly-timed CF-Poll packets
 *  - reclaim the medium during a HCCA-txop if 
 *    the txop is unused.
 *
 * All of this is done by a close cooperation with the MacLow and the Phy.
 * The first mission of the scheduler is to generate the beacons: when
 * the scheduler is started, it schedules a beacon timeout. When the beacon
 * timeout expires, the scheduler verifies that the medium has been idle 
 * for at least PIFS. If not, it reschedules the beacon timeout for 
 * the expected end-of-idle+PIFS which should ensure that it wins access
 * to the medium even in case of contention. If yes, it programs the MacLow
 * to tx a beacon and re-schedules a beacon timeout for the next beacon.
 *
 * When end-of-beacon-tx+SIFS happens, we need to start a CAP:
 *   - tx CF-Poll
 *   - enable "busy monitoring"
 *   - schedule a start-of-cap event for the next CAP only if can
 *     complete before the next start-of-beacon event.
 *
 * When start-of-cap happens, we verify that the medium has been idle
 * for at least PIFS. If not, we re-schedule start-of-cap for 
 * expected-end-of-busy+PIFS. If the medium is ok, we do the same
 * thing as end-of-beacon-tx.
 *
 * If, at any time during a CAP, we get a busy-timeout event, it means
 * that we can reclaim the medium for the next txop so we tx the next 
 * CF-Poll. This busy-timeout is used to detect the end-of-txop event.
 * and start the next txop of a CAP if there is one.
 */

#include "qap-scheduler.h"
#include "mac-container.h"
#include "mac-parameters.h"
#include "mac-dcf-parameters.h"
#include "mac-high-qap.h"
#include "mac-low.h"
#include "tspec.h"
#include "dcf.h"

Txop::Txop (int destination, TSpec *tspec)
	: m_destination (destination),
	  m_tspec (tspec)
{}

int
Txop::getDestination (void) const
{
	return m_destination;
}

TSpec *
Txop::getTSpec (void) const
{
	return m_tspec;
}

bool
Txop::operator == (Txop const & o) const
{
	// XXX this is broken. ohmy.
	if (m_tspec->getTSID () == o.m_tspec->getTSID () &&
	    getDestination () == o.getDestination ()) {
		return true;
	} else {
		return false;
	}
}


class MyMacLowBusyMonitoringListener : public MacLowBusyMonitoringListener {
public:
	MyMacLowBusyMonitoringListener (QapScheduler *scheduler) 
		: m_scheduler (scheduler)
	{}

	virtual ~MyMacLowBusyMonitoringListener () {}

	virtual void gotBusyTimeout (void) {
		m_scheduler->busyTimeout ();
	}

private:
	QapScheduler *m_scheduler;
};

class MyMacLowTransmissionListener : public MacLowTransmissionListener {
public:
	MyMacLowTransmissionListener (QapScheduler *scheduler)
		: m_scheduler (scheduler)
	{}
	virtual ~MyMacLowTransmissionListener () {}

	virtual void gotCTS (double snr, int txMode) {}
	virtual void missedCTS (void) {}
	virtual void gotACK (double snr, int txMode) {}
	virtual void missedACK (void) {}
	virtual void startNext (void) {
		m_scheduler->beaconTxNextData ();
	}
private:
	QapScheduler *m_scheduler;
};


QapScheduler::QapScheduler (MacContainer *container)
	: m_container (container),
	  m_sequence (0),
	  m_currentServiceInterval (0.0)
{
	m_busyListener = new MyMacLowBusyMonitoringListener (this);
	m_beaconTxListener = new MyMacLowTransmissionListener (this);
	m_access = new DynamicHandler<QapScheduler> (this, &QapScheduler::accessTimer);
	m_beacon = new DynamicHandler<QapScheduler> (this, &QapScheduler::beaconTimer);
	m_beacon->start (parameters ()->getBeaconInterval ());

	m_dcfParameters[AC_BE] = new MacDcfParameters (container);
	m_dcfParameters[AC_BK] = new MacDcfParameters (container);
	m_dcfParameters[AC_VI] = new MacDcfParameters (container);
	m_dcfParameters[AC_VO] = new MacDcfParameters (container);

	/* default parameters from section 7.3.2.14 802.11e/D12.1*/
	uint16_t CWmin;
	uint16_t CWmax;
	if (true) {
		/* 802.11a */
		CWmin = 15;
		CWmax = 1023;
		m_dcfParameters[AC_VO]->setTxopLimit (1.504e-3);
		m_dcfParameters[AC_VI]->setTxopLimit (3.008e-3);
	} else {
		/* 802.11b */
		CWmin = 31;
		CWmax = 1023;
		m_dcfParameters[AC_VO]->setTxopLimit (3.264e-3);
		m_dcfParameters[AC_VI]->setTxopLimit (6.016e-3);
	}

	m_dcfParameters[AC_BK]->setACM (false);
	m_dcfParameters[AC_BK]->setAIFSN (7);
	m_dcfParameters[AC_BK]->setCWmin (CWmin);
	m_dcfParameters[AC_BK]->setCWmax (CWmax);
	m_dcfParameters[AC_BK]->setTxopLimit (0.0);

	m_dcfParameters[AC_BE]->setACM (false);
	m_dcfParameters[AC_BE]->setAIFSN (3);
	m_dcfParameters[AC_BE]->setCWmin (CWmin);
	m_dcfParameters[AC_BE]->setCWmax (CWmax);
	m_dcfParameters[AC_BE]->setTxopLimit (0.0);

	m_dcfParameters[AC_VI]->setACM (false);
	m_dcfParameters[AC_VI]->setAIFSN (2);
	m_dcfParameters[AC_VI]->setCWmin ((CWmin+1)/2-1);
	m_dcfParameters[AC_VI]->setCWmax ((CWmin));

	m_dcfParameters[AC_VO]->setACM (false);
	m_dcfParameters[AC_VO]->setAIFSN (2);
	m_dcfParameters[AC_VO]->setCWmin ((CWmin+1)/4-1);
	m_dcfParameters[AC_VO]->setCWmax ((CWmin+1)/2-1);

}

void
QapScheduler::storeEdcaParametersInPacket (Packet *packet)
{
	packet->allocdata (4*4);
	unsigned char *buffer = packet->accessdata ();

	for (uint8_t ac = 0; ac < 4; ac++) {
		m_dcfParameters[ac]->writeTo (buffer, (enum ac_e)ac);
		buffer += 4;
	}
}

Dcf *
QapScheduler::createDcf (enum ac_e ac)
{
	MacDcfParameters *dcfParameters = new MacDcfParameters (m_container);
	*dcfParameters = *(m_dcfParameters[ac]);
	Dcf *dcf = new Dcf (m_container, dcfParameters);
	return dcf;
}


Packet *
QapScheduler::getPacketFor (int destination)
{
	Packet *packet = Packet::alloc ();
	setSource (packet, m_container->selfAddress ());
	setFinalDestination (packet, destination);
	setDestination (packet, destination);
	return packet;
}

int
QapScheduler::getPacketSize (enum mac_80211_packet_type type)
{
	return parameters ()->getPacketSize (type);
}

Phy80211 *
QapScheduler::phy (void)
{
	return m_container->phy ();
}

MacLow *
QapScheduler::low (void)
{
	return m_container->macLow ();
}

double
QapScheduler::now (void)
{
	return Scheduler::instance ().clock ();
}

MacParameters *
QapScheduler::parameters (void)
{
	return m_container->parameters ();
}

double
QapScheduler::duration (int size, int mode)
{
	double txDuration = m_container->phy ()->calculateTxDuration (mode, size);
	return txDuration;
}

double
QapScheduler::calculateMediumTime (TSpec const *tspec)
{
	// see I.2.2 802.11e/D12.1
	double pps = ceil (tspec->getMeanDataRate () / 8) / tspec->getNominalMSDUSize ();
	// see 9.9.3.1.2 802.11e/D12.1
	double mpduExchangeTime = 0.0;
	mpduExchangeTime += duration (tspec->getNominalMSDUSize (), tspec->getMinimumPhyMode ());
	mpduExchangeTime += duration (getPacketSize (MAC_80211_CTL_ACK), 0);
	mpduExchangeTime += parameters ()->getSIFS ();
	double mediumTime = tspec->getSurplusBandwidthAllowance () * pps * mpduExchangeTime;
	return mediumTime;
}

double
QapScheduler::calculateTxopDuration (double serviceInterval, TSpec const *tspec)
{
	/* see I.3.3.1 802.11e/D12.1 */
	long int N = lrint (serviceInterval * tspec->getMeanDataRate () / tspec->getNominalMSDUSize ());
	double overhead = 0.0;
	overhead += duration (getPacketSize (MAC_80211_MGT_CFPOLL), 0);
	overhead += parameters ()->getSIFS ();
	overhead += duration (getPacketSize (MAC_80211_CTL_ACK), 0) * N;
	overhead += parameters ()->getSIFS () * N;
	double txTime = 0.0;
	txTime += duration (N * tspec->getNominalMSDUSize (), tspec->getMinimumPhyMode ());
	txTime += overhead;
	double otherTxTime = 0.0;
	otherTxTime += duration (parameters ()->getMaxMSDUSize (), tspec->getMinimumPhyMode ());
	otherTxTime += overhead;
	double txop = max (txTime, otherTxTime);
	return txop;
}

double
QapScheduler::getCurrentServiceInterval (void)
{
	return m_currentServiceInterval;
}

void
QapScheduler::setCurrentServiceInterval (double serviceInterval)
{
	m_currentServiceInterval = serviceInterval;
}

double
QapScheduler::getMaximumServiceInterval (TSpec *tspec)
{
	double maximumServiceInterval;
	if (tspec->getMaximumServiceInterval () <= 0.0) {
		assert (tspec->getDelayBound () > 0.0);
		// XXX I have no idea what value we should use here.
		int numberOfRetries = 7;
		maximumServiceInterval = tspec->getDelayBound () / numberOfRetries;
	} else {
		maximumServiceInterval = tspec->getMaximumServiceInterval ();
	}
	return maximumServiceInterval;
}

double
QapScheduler::calculateTotalCapTime (double serviceInterval)
{
	double capTime = 0.0;
	list<Txop>::const_iterator i;
	for (i = m_admitted.begin (); i != m_admitted.end (); i++) {
		capTime += calculateTxopDuration (serviceInterval, (*i).getTSpec ());
	}
	assert (capTime <= getCurrentServiceInterval ());
	return capTime;
}

double
QapScheduler::getCurrentTotalCapTime (void)
{
	return calculateTotalCapTime (getCurrentServiceInterval ());
}

bool
QapScheduler::addTsRequest (TSpec *tspec)
{
	/* make sure we have a minimum set of parameters.
	 */
	if (tspec->getMeanDataRate () <= 0.0 ||
	    tspec->getNominalMSDUSize () == 0 ||
	    tspec->getSurplusBandwidthAllowance () <= 0.0 ||
	    (tspec->getMaximumServiceInterval () <= 0.0 &&
	     tspec->getDelayBound () <= 0.0)) {
		/* we don't have a minimum set of parameters
		 * send back invalid ADDTS_RESPONSE.
		 * see section 9.9.3.2 802.11e/D12.1
		 */
		return false;
	}
	tspec->setMediumTime (calculateMediumTime (tspec));
	double maximumServiceInterval = getMaximumServiceInterval (tspec);
	double newServiceInterval;
	if (getCurrentServiceInterval () > 0.0 &&
	    getCurrentServiceInterval () < maximumServiceInterval) {
		newServiceInterval = getCurrentServiceInterval ();
	} else {
		newServiceInterval = maximumServiceInterval;
	}
	double capTime = 0.0;
	capTime += calculateTxopDuration (newServiceInterval, tspec);
	capTime += calculateTotalCapTime (newServiceInterval);
	if (capTime > parameters ()->getCapLimit ()) {
		/* We cannot allow each CAP to be longer than CapLimit. */
		return false;
	}
	double capProportion = capTime / newServiceInterval;
	if (capProportion > parameters ()->getMinEdcaTrafficProportion ()) {
		/* we must allow at least a small proportion of EDCA traffic. */
		return false;
	}
	/* The new schedule should be okay. We need to advertise it now. */
	setCurrentServiceInterval (newServiceInterval);
	// need to move around source dest XXX
	m_admitted.push_back (Txop (0, tspec));
	return true;
}

bool
QapScheduler::delTsRequest (int destination, TSpec *tspec)
{
	/* We assume that pointer equality means tspec equality.
	 * This is wrong. XXX
	 */

	// remove TS.
	list<Txop>::iterator item = std::find (m_admitted.begin (), m_admitted.end (), 
					  Txop (destination, tspec));
	if (item == m_admitted.end ()) {
		/* Someone asked us to delete an admited tspec which
		 * is not admited. Weird.
		 */
		return false;
	}
	m_admitted.erase (item);


	// update schedule
	list<Txop>::const_iterator i;
	double minMaxServiceInterval = parameters ()->getBeaconInterval ();
	for (i = m_admitted.begin (); i != m_admitted.end (); i++) {
		double maxServiceInterval = (*i).getTSpec ()->getMaximumServiceInterval ();
		if (maxServiceInterval < minMaxServiceInterval) {
			minMaxServiceInterval = maxServiceInterval;
		}
	}
	if (minMaxServiceInterval != getCurrentServiceInterval ()) {
		/* service interval changed. */
		setCurrentServiceInterval (minMaxServiceInterval);
	}

	// update scheduling timers. XXX

	// notify client.
	return false;
}

void 
QapScheduler::gotQosNull (Packet *packet)
{

}

void
QapScheduler::doCurrentTxop (void)
{
	assert (m_txopIterator != m_admitted.end ());

	TSpec *tspec = (*m_txopIterator).getTSpec ();

	double txopDuration = calculateTxopDuration (getCurrentServiceInterval (), tspec);
	m_txopEnd = now () + txopDuration;
	if (m_txopEnd >= m_capEnd) {
		/* we cannot finish all the txops during the CAP.
		 * This is probably because one of the txops stole
		 * some medium time.
		 * anyway, we just give up on this CAP.
		 */
		finishCap ();
		return;
	}
	
	int destination = (*m_txopIterator).getDestination ();
	Packet *packet = getPacketFor (destination);
	setSize (packet, getPacketSize (MAC_80211_MGT_CFPOLL));
	setType (packet, MAC_80211_MGT_CFPOLL);
	setAC (packet, AC_SPECIAL);
	setSequenceNumber (packet, m_sequence);
	setFragmentNumber (packet, 0);
	m_sequence++;
	m_sequence %= 4096;
	low ()->disableACK ();
	low ()->disableRTS ();
	low ()->enableOverrideDurationId (txopDuration);
	low ()->setDataTransmissionMode (0);
	low ()->disableNextData ();
	low ()->setData (packet);
	low ()->setTransmissionListener (NullMacLowTransmissionListener::instance ());
	low ()->startTransmission ();	
}

void
QapScheduler::finishCap (void)
{
	low ()->disableBusyMonitoring ();
}

void
QapScheduler::startCap (void)
{
	m_capStart = now ();
	double capEnd = m_capStart + getCurrentTotalCapTime ();
	m_capEnd = min (capEnd, m_nextBeaconStart);
	m_txopIterator = m_admitted.begin ();

	low ()->setBusyMonitoringListener (m_busyListener);
	low ()->enableBusyMonitoring ();

	doCurrentTxop ();

	m_txopIterator++;

	/* Schedule an access for the next service interval. */
	double nextCapStart = now () + getCurrentServiceInterval ();
	if (nextCapStart < m_nextBeaconStart) {
		m_access->start (getCurrentServiceInterval ());
	}
}

void
QapScheduler::busyTimeout (void)
{
	doCurrentTxop ();
	m_txopIterator++;
	if (m_txopIterator == m_admitted.end ()) {
		finishCap ();
	}
}

void
QapScheduler::accessTimer (MacCancelableEvent *event)
{
	/* Invoked at the start of a CAP to send the first 
	 * CF-Poll of the CAP.
	 */
	// XXX: can we ignore the NAV here ?
	if (phy ()->getState () != Phy80211::IDLE ||
	    phy ()->getStateDuration () < parameters ()->getPIFS ()) {
		/* Somehow, we got delayed so we need to wait
		 * until next idle+PIFS.
		 */
		double nextAccess = now ();
		nextAccess += phy ()->getDelayUntilIdle ();
		nextAccess += parameters ()->getPIFS ();
		m_access->start (nextAccess);
		return;
	}

	/* Access has been granted to us. */
	startCap ();
}

void
QapScheduler::beaconTxNextData (void)
{
	/* Invoked SIFS after Beacon is transmitted.
	 * Used to start transmission of the first CF-Poll
	 * after a Beacon.
	 */
	if (m_admitted.empty ()) {
		/* well, no CF-Poll to transmit. */
		return;
	}
	if (phy ()->getState () != Phy80211::IDLE) {
		double nextAccess = now ();
		nextAccess += phy ()->getDelayUntilIdle ();
		nextAccess += parameters ()->getPIFS ();
		m_access->start (nextAccess);
		return;
	}

	startCap ();
}

void
QapScheduler::beaconTimer (MacCancelableEvent *event)
{
	if (phy ()->getState () != Phy80211::IDLE ||
	    phy ()->getStateDuration () < parameters ()->getPIFS ()) {
		/* Somehow, we got delayed so we need to wait
		 * until next idle+PIFS.
		 */
		double nextBeacon = now ();
		nextBeacon += phy ()->getDelayUntilIdle ();
		nextBeacon += parameters ()->getPIFS ();
		m_beacon->start (nextBeacon);
		return;		
	}
	
	Packet *packet = getPacketFor (MAC_BROADCAST);
	setSize (packet, getPacketSize (MAC_80211_MGT_BEACON));
	setType (packet, MAC_80211_MGT_BEACON);
	storeEdcaParametersInPacket (packet);
	setAC (packet, AC_SPECIAL);
	setSequenceNumber (packet, m_sequence);
	setFragmentNumber (packet, 0);
	m_sequence++;
	m_sequence %= 4096;
	low ()->disableACK ();
	low ()->disableRTS ();
	low ()->disableOverrideDurationId ();
	low ()->setDataTransmissionMode (0);
	if (m_admitted.empty ()) {
		low ()->disableNextData ();
	} else {
		low ()->enableNextData (getPacketSize (MAC_80211_MGT_CFPOLL));
	}
	low ()->setData (packet);
	low ()->setTransmissionListener (m_beaconTxListener);
	low ()->startTransmission ();

	/* schedule an access for the next beacon. */
	m_beacon->start (parameters ()->getBeaconInterval ());
}
