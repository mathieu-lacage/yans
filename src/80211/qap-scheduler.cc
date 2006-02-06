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
#include "mac-parameters.h"
#include "mac-dcf-parameters.h"
#include "mac-high-qap.h"
#include "mac-low.h"
#include "tspec.h"
#include "dcf.h"
#include "mac-tx-middle.h"
#include "net-interface-80211.h"
#include "common.h"
#include "phy-80211.h"

#include "packet.h"


#ifndef QAP_SCHEDULER_TRACE
#define QAP_SCHEDULER_TRACE 1
#endif /* QAP_SCHEDULER_TRACE */

#ifdef QAP_SCHEDULER_TRACE
# define TRACE(format, ...) \
	printf ("SCHED TRACE %d %f " format "\n", m_interface->getMacAddress (), \
                Scheduler::instance ().clock (), ## __VA_ARGS__);
#else /* QAP_SCHEDULER_TRACE */
# define TRACE(format, ...)
#endif /* QAP_SCHEDULER_TRACE */


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



class MyBeaconMacLowTransmissionListener : public MacLowTransmissionListener {
public:
	MyBeaconMacLowTransmissionListener (QapScheduler *scheduler)
		: m_scheduler (scheduler)
	{}
	virtual ~MyBeaconMacLowTransmissionListener () {}

	virtual void gotCTS (double snr, int txMode) {
		assert (false);
	}
	virtual void missedCTS (void) {
		assert (false);
	}
	virtual void gotACK (double snr, int txMode) {
		assert (false);
	}
	virtual void missedACK (void) {
		assert (false);
	}
	virtual void startNext (void) {
		m_scheduler->beaconTxNextData ();
	}
	virtual void cancel (void) {
		assert (false);
	}
private:
	QapScheduler *m_scheduler;
};
class MyCfPollMacLowTransmissionListener : public MacLowTransmissionListener {
public:
	MyCfPollMacLowTransmissionListener (QapScheduler *scheduler)
		: m_scheduler (scheduler)
	{}
	virtual ~MyCfPollMacLowTransmissionListener () {}

	virtual void gotCTS (double snr, int txMode) {
		assert (false);
	}
	virtual void missedCTS (void) {
		assert (false);
	}
	virtual void gotACK (double snr, int txMode) {
		m_scheduler->gotCfPollAck ();
	}
	virtual void missedACK (void) {
		m_scheduler->missedCfPollAck ();
	}
	virtual void startNext (void) {
		assert (false);
	}
	virtual void cancel (void) {
		assert (false);
	}
private:
	QapScheduler *m_scheduler;
};


QapScheduler::QapScheduler ()
	: m_currentServiceInterval (0.0),
	  m_sequence (0)
{
	m_beaconTxListener = new MyBeaconMacLowTransmissionListener (this);
	m_cfPollTxListener = new MyCfPollMacLowTransmissionListener (this);
	m_capStart = new DynamicHandler<QapScheduler> (this, &QapScheduler::capStartTimer);
	m_beacon = new DynamicHandler<QapScheduler> (this, &QapScheduler::beaconTimer);
	m_txopStart = new DynamicHandler<QapScheduler> (this, &QapScheduler::txopStartTimer);

	m_dcfParameters[AC_BE] = new MacDcfParameters ();
	m_dcfParameters[AC_BK] = new MacDcfParameters ();
	m_dcfParameters[AC_VI] = new MacDcfParameters ();
	m_dcfParameters[AC_VO] = new MacDcfParameters ();

	m_dcfParameters[AC_BK]->setACM (false);
	m_dcfParameters[AC_BK]->setAIFSN (7);
	m_dcfParameters[AC_BK]->setTxopLimit (0.0);

	m_dcfParameters[AC_BE]->setACM (false);
	m_dcfParameters[AC_BE]->setAIFSN (3);
	m_dcfParameters[AC_BE]->setTxopLimit (0.0);

	m_dcfParameters[AC_VI]->setACM (false);
	m_dcfParameters[AC_VI]->setAIFSN (2);

	m_dcfParameters[AC_VO]->setACM (false);
	m_dcfParameters[AC_VO]->setAIFSN (2);

	set80211a ();
}

void
QapScheduler::updateCWboundaries (uint16_t CWmin, uint16_t CWmax)
{
	/* default parameters from section 7.3.2.14 802.11e/D12.1*/
	m_dcfParameters[AC_BK]->setCWmin (CWmin);
	m_dcfParameters[AC_BK]->setCWmax (CWmax);
	m_dcfParameters[AC_BE]->setCWmin (CWmin);
	m_dcfParameters[AC_BE]->setCWmax (CWmax);
	m_dcfParameters[AC_VI]->setCWmin ((CWmin+1)/2-1);
	m_dcfParameters[AC_VI]->setCWmax ((CWmin));
	m_dcfParameters[AC_VO]->setCWmin ((CWmin+1)/4-1);
	m_dcfParameters[AC_VO]->setCWmax ((CWmin+1)/2-1);
}

void
QapScheduler::set80211a (void)
{
	m_dcfParameters[AC_VO]->setTxopLimit (1.504e-3);
	m_dcfParameters[AC_VI]->setTxopLimit (3.008e-3);
	updateCWboundaries (15, 1023);
}

void
QapScheduler::set80211b (void)
{
	m_dcfParameters[AC_VO]->setTxopLimit (3.264e-3);
	m_dcfParameters[AC_VI]->setTxopLimit (6.016e-3);
	updateCWboundaries (31, 1023);
}

void 
QapScheduler::setInterface (NetInterface80211 *interface)
{
	m_interface = interface;

	MacParameters *parameters = interface->parameters ();
	m_beacon->start (parameters->getBeaconInterval ());

	m_dcfParameters[AC_BE]->setParameters (parameters);
	m_dcfParameters[AC_BK]->setParameters (parameters);
	m_dcfParameters[AC_VI]->setParameters (parameters);
	m_dcfParameters[AC_VO]->setParameters (parameters);
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
	MacDcfParameters *dcfParameters = new MacDcfParameters ();
	dcfParameters->setParameters (m_interface->parameters ());
	*dcfParameters = *(m_dcfParameters[ac]);
	Dcf *dcf = new Dcf (dcfParameters);
	dcf->setInterface (m_interface);
	return dcf;
}


Packet *
QapScheduler::getPacketFor (int destination)
{
	Packet *packet = hdr_mac_80211::create (m_interface->getMacAddress ());
	setFinalDestination (packet, destination);
	setDestination (packet, destination);
	return packet;
}

int
QapScheduler::getPacketSize (enum mac_80211_packet_type type)
{
	return parameters ()->getPacketSize (type);
}

MacLow *
QapScheduler::low (void)
{
	return m_interface->low ();
}

Phy80211 *
QapScheduler::phy (void)
{
	return m_interface->phy ();
}

MacParameters *
QapScheduler::parameters (void)
{
	return m_interface->parameters ();
}


double
QapScheduler::now (void)
{
	return Scheduler::instance ().clock ();
}

double
QapScheduler::duration (int size, int mode)
{
	double txDuration = phy ()->calculateTxDuration (mode, size);
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
QapScheduler::getMaxTxopDuration (void)
{
	double maxTxop = 0xff * 32.0e-6;
	return maxTxop;
}

double
QapScheduler::getCurrentTotalCapTime (void)
{
	return calculateTotalCapTime (getCurrentServiceInterval ());
}

bool
QapScheduler::addTsRequest (int destination, TSpec *tspec)
{
	/* make sure we have a minimum set of parameters.
	 */
	if (tspec->getMeanDataRate () <= 0.0 ||
	    tspec->getNominalMSDUSize () == 0 ||
	    tspec->getSurplusBandwidthAllowance () <= 0.0 ||
	    (tspec->getMaximumServiceInterval () <= 0.0 &&
	     tspec->getDelayBound () <= 0.0) ||
	    (tspec->getMaximumServiceInterval () < 
	     tspec->getMinimumServiceInterval ())) {
		/* we don't have a minimum set of parameters
		 * or we have invalid parameters.
		 * send back invalid ADDTS_RESPONSE.
		 * see section 9.9.3.2 802.11e/D12.1
		 */
		TRACE ("Refused new stream. Missing minimum set of parameters");
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
	double txopDuration = calculateTxopDuration (newServiceInterval, tspec);
	while (txopDuration > getMaxTxopDuration () &&
	       newServiceInterval > 0.008) {
		/* The txop duration for this TS is larger than 
		 * what can be encoded in the txoplimit of the cfpoll 
		 * packet.
		 * We need to adjust the service interval to decrease
		 * the length of the TXOP.
		 */
		newServiceInterval -= 0.001;
		txopDuration = calculateTxopDuration (newServiceInterval, tspec);
	}
	if (newServiceInterval <= 0.008) {
		TRACE ("Refused new stream. Something is wrong in the tspec parameters.\n"
		       "\tCalculated SI: %f txop: %f max txop: %f",
		       newServiceInterval,
		       txopDuration,
		       getMaxTxopDuration ());
		return false;
	}
	capTime += txopDuration;
	capTime += calculateTotalCapTime (newServiceInterval);
	if (capTime > parameters ()->getCapLimit ()) {
		/* We cannot allow each CAP to be longer than CapLimit. */
		TRACE ("Refused new stream. Cap time too big: %f > %f",
		       capTime,
		       parameters ()->getCapLimit ());
		return false;
	}
	double capProportion = capTime / newServiceInterval;
	if (capProportion > parameters ()->getMinEdcaTrafficProportion ()) {
		/* we must allow at least a small proportion of EDCA traffic. */
		TRACE ("Refused new stream. Cap proportion too big: %f > %f",
		       capProportion,
		       parameters ()->getMinEdcaTrafficProportion ());
		return false;
	}
	/* The new schedule should be okay. We need to advertise it now. */
	setCurrentServiceInterval (newServiceInterval);
	TRACE ("Admitted new stream. Service interval: %f, capTime: %f", newServiceInterval, capTime);
	// need to move around source dest XXX
	m_admitted.push_back (Txop (destination, tspec));
	return true;
}

bool
QapScheduler::delTsRequest (int destination, TSpec *tspec)
{
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

	/* XXX The scheduling timers will be updated at the 
	 * next Beacon boundary. Maybe we need to trigger an update
	 * of the timers now.
	 */
	

	// notify client.
	return false;
}

void 
QapScheduler::gotQosNull (Packet *packet)
{
	TRACE ("got QosNull from %d", getSource (packet));
	/* The txop holder notifies us that it has
	 * finished using the medium so we can reuse
	 * any txop time left for other txops. Thus,
	 * we schedule a start txop for end-of-ack-tx+SIFS
	 */
	m_txopStart->cancel ();
	double nextTxopStartDelay = 0.0;
	nextTxopStartDelay += getDuration (packet);
	nextTxopStartDelay += parameters ()->getPIFS ();
	m_txopStart->start (nextTxopStartDelay);
}
void 
QapScheduler::gotCfPollAck (void)
{
	TRACE ("got CfPoll Ack from %d", (*m_txopIterator).getDestination ());
	/* We don't have anything to do here since this event 
	 * notifies us of the fact that the target QSTA of the 
	 * txop has effectively taken ownership of the medium
	 * for the duration of the txop.
	 * So, we wait for either a QosNull event or the 
	 * end-of-txop event.
	 */
}
void 
QapScheduler::missedCfPollAck (void)
{
	TRACE ("missed CfPoll Ack from %d", (*m_txopIterator).getDestination ());
	/* Our target QSTA has not taken ownership of the 
	 * medium so we skip to the next txop if there is
	 * one.
	 */
	nextTxop ();
}

void
QapScheduler::startCurrentTxop (void)
{
	assert (m_txopIterator != m_admitted.end ());

	TSpec *tspec = (*m_txopIterator).getTSpec ();

	double txopDuration = calculateTxopDuration (getCurrentServiceInterval (), tspec);
	m_txopEndTime = now () + txopDuration;
	if (m_txopEndTime > m_capEndTime) {
		TRACE ("impossible to finish txop before end of CAP");
		/* we cannot finish all the txops during the CAP.
		 * This is probably because one of the txops stole
		 * some medium time.
		 * anyway, we just give up on this CAP.
		 */
		finishCap ();
		return;
	}

	TRACE ("start txop for %d until %f", (*m_txopIterator).getDestination (), m_txopEndTime);
	
	low ()->enableSuperFastAck ();
	sendCfPollTo ((*m_txopIterator).getDestination (), 
		      tspec->getTSID (),
		      txopDuration);

	/* XXX start a timer for end-of-txop. */
}

void
QapScheduler::nextTxop (void)
{
	m_txopIterator++;
	if (m_txopIterator == m_admitted.end ()) {
		finishCap ();
	} else {
		startCurrentTxop ();
		std::list<Txop>::const_iterator i_copy (m_txopIterator);
		i_copy++;
		if (i_copy != m_admitted.end ()) {
			m_txopStart->start (m_txopEndTime - now () + parameters ()->getPIFS ());
		}
	}
}

void
QapScheduler::sendCfPollTo (int destination, uint8_t tsid, double txopDuration)
{
	Packet *packet = getPacketFor (destination);
	setSize (packet, getPacketSize (MAC_80211_MGT_CFPOLL));
	setType (packet, MAC_80211_MGT_CFPOLL);
	setTxopLimit (packet, txopDuration);
	setNoAck (packet);
	setTID (packet, tsid);
	setFragmentNumber (packet, 0);
	uint16_t sequence = m_interface->txMiddle ()->getNextSequenceNumberFor (packet);
	setSequenceNumber (packet, sequence);
	low ()->disableRTS ();
	low ()->enableOverrideDurationId (txopDuration);
	low ()->setDataTransmissionMode (0);
	low ()->disableNextData ();
	low ()->setData (packet);
	low ()->setTransmissionListener (m_cfPollTxListener);
	low ()->startTransmission ();	
}

void
QapScheduler::finishCap (void)
{
	TRACE ("finish CAP");

	/* notify all stations that the previous txop was finished 
	 * before its normal end such that contention-based access
	 * can resume.
	 */
	double cfPollDuration = phy ()->calculateTxDuration (0, getPacketSize (MAC_80211_MGT_CFPOLL));
	double cfPollEnd = now () + cfPollDuration;
	if (cfPollEnd < m_capEndTime) {
		low ()->disableACK ();
		sendCfPollTo (m_interface->getMacAddress (), 0, 0.0);
	}
}

void
QapScheduler::startCap (void)
{
	m_capStartTime = now ();
	double capEnd = m_capStartTime + getCurrentTotalCapTime ();
	TRACE ("start CAP until %f", capEnd);
	m_capEndTime = min (capEnd, m_nextBeaconStartTime);
	m_txopIterator = m_admitted.begin ();


	startCurrentTxop ();

	/* Schedule an access for the next service interval. */
	double nextCapStart = now () + getCurrentServiceInterval ();
	if (nextCapStart < m_nextBeaconStartTime) {
		m_capStart->start (getCurrentServiceInterval ());
	}
}

void
QapScheduler::txopStartTimer (MacCancelableEvent *event)
{
	// XXX: can we ignore the NAV here ?
	if (phy ()->getState () != Phy80211::IDLE ||
	    phy ()->getStateDuration () < parameters ()->getPIFS ()) {
		/* Somehow, we got delayed so we need to wait
		 * until next idle+PIFS.
		 */
		TRACE ("cannot start txop now. restart delay.");
		double nextTxopStartDelay = 0.0;
		nextTxopStartDelay += phy ()->getDelayUntilIdle ();
		nextTxopStartDelay += parameters ()->getPIFS ();
		m_txopStart->start (nextTxopStartDelay);
		return;
	}
	nextTxop ();
}

void
QapScheduler::capStartTimer (MacCancelableEvent *event)
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
		TRACE ("cannot start CAP now. restart delay. -- 1");
		double nextCapStartDelay = 0.0;
		nextCapStartDelay += phy ()->getDelayUntilIdle ();
		nextCapStartDelay += parameters ()->getPIFS ();
		m_capStart->start (nextCapStartDelay);
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
		TRACE ("cannot start CAP now. restart delay. -- 2");
		double nextCapStartDelay = 0.0;
		nextCapStartDelay += phy ()->getDelayUntilIdle ();
		nextCapStartDelay += parameters ()->getPIFS ();
		m_capStart->start (nextCapStartDelay);
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
		TRACE ("beacon delay. Must have PIFS idle.");
		double nextBeaconDelay = 0.0;
		nextBeaconDelay += phy ()->getDelayUntilIdle ();
		nextBeaconDelay += parameters ()->getPIFS ();
		m_beacon->start (nextBeaconDelay);
		return;		
	}

	TRACE ("sending beacon");
	
	Packet *packet = getPacketFor (MAC_BROADCAST);
	setSize (packet, getPacketSize (MAC_80211_MGT_BEACON));
	setType (packet, MAC_80211_MGT_BEACON);
	storeEdcaParametersInPacket (packet);
	setAC (packet, AC_SPECIAL);
	setFragmentNumber (packet, 0);
	uint16_t sequence = m_interface->txMiddle ()->getNextSequenceNumberFor (packet);
	setSequenceNumber (packet, sequence);
	low ()->disableACK ();
	low ()->disableRTS ();
	low ()->disableOverrideDurationId ();
	low ()->setDataTransmissionMode (0);
	if (m_admitted.empty ()) {
		low ()->disableNextData ();
	} else {
		low ()->enableNextData (getPacketSize (MAC_80211_MGT_CFPOLL), 0);
	}
	low ()->setData (packet);
	low ()->setTransmissionListener (m_beaconTxListener);
	low ()->startTransmission ();

	/* schedule an access for the next beacon. */
	m_beacon->start (parameters ()->getBeaconInterval ());
	m_nextBeaconStartTime = now () + parameters ()->getBeaconInterval ();
}
