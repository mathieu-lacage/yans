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

#include "phy-80211.h"
#include "bpsk-mode.h"
#include "qam-mode.h"
#include "rng-uniform.h"
#include "hdr-mac-80211.h"

#include "antenna.h"
#include "propagation.h"
#include "transmission-mode.h"


#include <iostream>
#include <vector>
#include <algorithm>
#include <cfloat>

#define nopePHY80211_DEBUG 1
#define nopePHY80211_STATE_DEBUG 1

/* J/K */
#define BOLTZMANN 1.3803e-23
/* W = J/s */

/* All the state transitions are marked by these macros. */
#ifdef PHY80211_STATE_DEBUG
#  define STATE_FROM(from) \
     printf ("PHY TRACE %d from %s", this->node_->address (), stateToString (from));
#  define STATE_TO(to) \
     printf (" to %s", stateToString (to));
#  define STATE_AT(at) \
     printf (" at %g\n", at);
#else
#  define STATE_FROM(from)
#  define STATE_TO(from)
#  define STATE_AT(at)
#endif

/****************************************************************
 *       TCL glue.
 ****************************************************************/

static class Phy80211Class: public TclClass {
public:
        Phy80211Class() : TclClass("Phy/Phy80211") {}
        TclObject* create(int, const char*const*) {
                return (new Phy80211 ());
        }
} class_Phy80211;

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/

Phy80211Listener::~Phy80211Listener ()
{}

/****************************************************************
 *       Phy event class
 ****************************************************************/

class PhyRxEvent {
public:
	PhyRxEvent (Packet *packet, class Phy80211 *phy)
	{
		m_payloadMode = phy->getPayloadMode (packet);
		m_size = ::getSize (packet);
		m_startTime = phy->now ();
		m_endTime = m_startTime + phy->calculatePacketDuration (getHeaderMode (),
									m_payloadMode,
									m_size);
		m_power = phy->calculatePower (packet);
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

class EndRxHandler : public Handler
{
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
	typedef void (Phy80211::*RxHandler)(class PhyRxEvent *event, Packet *packet);
	EndRxHandler (Phy80211 *phy, RxHandler handler)
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
	Phy80211 *m_phy;
	RxHandler m_handler;
	RxEvent *m_runningEvent;
};

/****************************************************************
 *       Class which records SNIR change events for a 
 *       short period of time.
 ****************************************************************/


Phy80211::NIChange::NIChange (double time, double delta)
	: m_time (time), m_delta (delta)
{}

double 
Phy80211::NIChange::getTime (void)
{
	return m_time;
}
double
Phy80211::NIChange::getDelta (void)
{
	return m_delta;
}

bool
Phy80211::NIChange::operator < (NIChange a) const
{
	return m_time < a.m_time;
}

/****************************************************************
 *       The actual Phy80211 class
 ****************************************************************/

Phy80211::Phy80211 ()
	: Phy (),
	  m_propagation (0),
	  m_antenna (0),
	  m_endRxHandler (new EndRxHandler (this, &Phy80211::endRx)),
	  m_random (new RngUniform ()),
	  m_sleeping (false),
	  m_rxing (false),
	  m_endTx (0.0),
	  m_previousStateChangeTime (0.0)
{
	if (isClassDefined ("Phy/Phy80211", "standard")) {
		bind ("standard", (unsigned int *)&m_standard);
	} else {
		m_standard = 2;
	}
	if (m_standard == 0) {
		cout << "nothing" << endl;
	} else if (m_standard == 1) {
		cout << "802.11b" << endl;
		// XXX
		m_frequency = 2.4e9;
		m_plcpPreambleDelay = 144e-6;
		m_plcpHeaderLength = 48;
	} else if (m_standard == 2) {
		cout << "802.11a" << endl;
		m_frequency = 5e9;
		m_plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16 + 6;
		m_plcpPreambleDelay = 20e-6;
		/* 4095 bytes at a 6Mb/s rate with a 1/2 coding rate. */
		m_maxPacketDuration = 4095.0*8.0/6000000.0*(1.0/2.0);
		addTxRxMode (new BPSKFECMode (10, 11,       20e6, 6e6, 0.5));
		addTxRxMode (new BPSKFECMode (5, 8,         20e6, 9e6, 0.75));
		addTxRxMode (new QAMFECMode (4, 10, 11, 0,  20e6, 12e6, 0.5));
		addTxRxMode (new QAMFECMode (4, 5, 8, 31,   20e6, 18e6, 0.75));
		addTxRxMode (new QAMFECMode (16, 10, 11, 0, 20e6, 24e6, 0.5));
		addTxRxMode (new QAMFECMode (16, 5, 8, 31,  20e6, 36e6, 0.75));
		addTxRxMode (new QAMFECMode (64, 6, 1, 16,  20e6, 48e6, 0.666));
		addTxRxMode (new QAMFECMode (64, 5, 8, 31,  20e6, 54e6, 0.75));
	}

}

Phy80211::~Phy80211 ()
{
	delete m_endRxHandler;
	delete m_random;
	list<PhyRxEvent *>::iterator i = m_rxEventList.begin ();
	while (i != m_rxEventList.end ()) {
		(*i)->unref ();
		i = m_rxEventList.erase (i);
	}

}

void 
Phy80211::registerListener (Phy80211Listener *listener)
{
	m_listeners.push_back (listener);
}

void 
Phy80211::notifyRxStart (double now, double duration)
{
	vector<Phy80211Listener *>::const_iterator i;
	for (i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyRxStart (now, duration);
	}
}
void 
Phy80211::notifyRxEnd (double now, bool receivedOk)
{
	vector<Phy80211Listener *>::const_iterator i;
	for (i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyRxEnd (now, receivedOk);
	}
}
void 
Phy80211::notifyTxStart (double now, double duration)
{
	vector<Phy80211Listener *>::const_iterator i;
	for (i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyTxStart (now, duration);
	}
}
void 
Phy80211::notifySleep (double now)
{
	vector<Phy80211Listener *>::const_iterator i;
	for (i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifySleep (now);
	}
}
void 
Phy80211::notifyWakeup (double now)
{
	vector<Phy80211Listener *>::const_iterator i;
	for (i = m_listeners.begin (); i != m_listeners.end (); i++) {
		(*i)->notifyWakeup (now);
	}
}


char const *
Phy80211::stateToString (enum Phy80211State state)
{
	switch (state) {
	case Phy80211::SYNC:
		return "SYNC";
		break;
	case Phy80211::TX:
		return "TX";
		break;
	case Phy80211::IDLE:
		return "IDLE";
		break;
	case Phy80211::SLEEP:
		return "SLEEP";
		break;
	}
	return "deadbeaf";
}

bool
Phy80211::isClassDefined (char const *className, char const *varName)
{
	Tcl& tcl=Tcl::instance();
	tcl.evalf ("catch \"%s set %s\" val", className, varName);
	if (strcmp ((char *)tcl.result (), "0") == 0) {
		return true;
	} else {
		return false;
	}
}

bool
Phy80211::isDefined (char const *varName)
{
	Tcl& tcl=Tcl::instance();
	tcl.evalf ("catch \"[%s info class] set %s\" val", (char *)name (), varName);
	if (strcmp ((char *)tcl.result (), "0") == 0) {
		return true;
	} else {
		return false;
	}
}

void
Phy80211::delay_bind_safe (char const *varName)
{
	if (isDefined (varName)) {
		delay_bind_init_one (varName);
	}
}

int 
Phy80211::delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer)
{
	if (delay_bind (varName, localName, "frequency", &m_frequency, tracer)) {
		goto ok;
	} else if (delay_bind (varName, localName, "plcpHeaderLength", &m_plcpHeaderLength, tracer)) {
		goto ok;
	} else if (delay_bind (varName, localName, "plcpPreambleDelay", &m_plcpPreambleDelay, tracer)) {
		goto ok;
	} else if (delay_bind (varName, localName, "systemLoss", &m_systemLoss, tracer)) {
		goto ok;
	} else if (delay_bind (varName, localName, "rxThreshold", &m_rxThreshold, tracer)) {
		goto ok;
	} else if (delay_bind (varName, localName, "rxNoise", &m_rxNoise, tracer)) {
		goto ok;
	}
	// chain up.
	return Phy::delay_bind_dispatch (varName, localName, tracer);
 ok:
	return TCL_OK;
}

void 
Phy80211::delay_bind_init_all()
{
	delay_bind_safe ("frequency");           // Hz
	delay_bind_safe ("plcpHeaderLength");    // bits
	delay_bind_safe ("plcpPreambleDelay");   // seconds

	/* absolute system loss. W. */
	delay_bind_safe  ("systemLoss");
	m_systemLoss = 1.0;
	/* absolute reception threshold. dBm. */
	delay_bind_safe ("rxThreshold");
	m_rxThreshold = -81;
	/* Ratio of energy lost by receiver. dB. */
	delay_bind_safe ("rxNoise");     
	m_rxNoise = 7;

	/* absolute transmission energy. dBm. 
	 * XXX: this field should be moved to the 
	 * packet structure.
	 */
	m_txPower = 15;

	// chain up.
	Phy::delay_bind_init_all ();
}

double 
Phy80211::dBmToW (double dBm)
{
	double mW = pow(10.0,dBm/10.0);
	return mW / 1000.0;
}
double 
Phy80211::dBToRatio (double dB)
{
	double ratio = pow(10.0,dB/10.0);
	return ratio;
}
double 
Phy80211::getLastRxSNR (void)
{
	return m_rxStartSNR;
}
double 
Phy80211::getLastRxStartTime (void)
{
	return m_rxStartTime;
}
int 
Phy80211::getNModes (void)
{
	return m_modes.size ();
}
double
Phy80211::now (void)
{
	double now;
	now = Scheduler::instance ().clock ();
	return now;
}
int
Phy80211::getHeaderMode (Packet *packet)
{
	return 0;
}
int
Phy80211::getPayloadMode (Packet *packet)
{
	return HDR_MAC_80211 (packet)->getTxMode ();
}
double 
Phy80211::getPreambleDuration (void)
{
	return m_plcpPreambleDelay;
}
double 
Phy80211::getLambda (void)
{	
	double lambda = SPEED_OF_LIGHT / m_frequency;
	return lambda;
}
double 
Phy80211::getSystemLoss (void)
{
	return m_systemLoss;
}

void
Phy80211::addTxRxMode (TransmissionMode *mode)
{
	m_modes.push_back (mode);
}

int
Phy80211::command(int argc, const char*const* argv)
{
	TclObject *obj; 


	if (argc == 3) {
		if( (obj = TclObject::lookup(argv[2])) == 0) {
			fprintf(stderr,"Phy80211: %s lookup of %s failed\n", 
				argv[1], argv[2]);
			return TCL_ERROR;
		} else if (strcmp (argv[1], "propagation") == 0) {
			assert (m_propagation == 0);
			m_propagation = static_cast<Propagation *> (obj);
			return TCL_OK;
		} else if (strcasecmp (argv[1], "antenna") == 0) {
			assert (m_antenna == 0);
			m_antenna = static_cast<Antenna *> (obj);
			return TCL_OK;
		}
	}
	return Phy::command(argc,argv);
}

void
Phy80211::recv(Packet *packet, Handler *)
{
	switch(HDR_CMN (packet)->direction ()) {
	case hdr_cmn::DOWN :
		startTx (packet);
		return;
	case hdr_cmn::UP :
		startRx (packet);
		break;
	default:
		assert (false);
	}
}
/* this two methods used to be called by the parent Phy
 * from Phy::recv but since we override Phy::recv,
 * we can implement empty stubs for these methods since
 * they will never be called.
 */
void Phy80211::sendDown(Packet *packet)
{}
int Phy80211::sendUp(Packet *packet)
{ return 0; }

void 
Phy80211::sleep (void)
{
	/* sleeping should never be tried if we are transmitting data
	 * because it is not possible to stop the reception of
	 * the corresponding packet for the nodes connected to this
	 * wireless medium. It would result in incorrect simulations.
	 */
	assert (getState () != Phy80211::TX);
	if (getState () == Phy80211::SYNC) {
		m_endRxHandler->cancel ();
	}
	notifySleep (now ());
	switchToSleep ();
}

void
Phy80211::wakeup (void)
{
	/* Obviously, wakeup should happen only if we
	 * went to sleep earlier.
	 */
	assert (getState () == Phy80211::SLEEP);
	notifyWakeup (now ());
	switchToIdleFromSleep ();
}




double
Phy80211::calculateHeaderDuration (int headerMode)
{
	return m_plcpHeaderLength / getMode (headerMode)->getDataRate ();
}

double
Phy80211::calculatePacketDuration (int headerMode, int payloadMode, int size)
{
	double delay = getPreambleDuration ();
	delay += calculateHeaderDuration (headerMode);
	delay += (size * 8) / getMode (payloadMode)->getDataRate ();
	return delay;
}

double 
Phy80211::calculateTxDuration (int payloadMode, int size)
{
	return calculatePacketDuration (0, payloadMode, size);
}

/* signalSpread: Hz or 1/s
 * returns Watts
 */
double
Phy80211::calculateNoiseFloor (double signalSpread)
{
	// thermal noise at 290K in J/s = W
	double Nt = BOLTZMANN * 290.0 * signalSpread;
	double Nr = dBToRatio (m_rxNoise);
	// receiver noise floor (W)
	return Nr * Nt;

}

/* signal and noiseInterference are both W
 */
double
Phy80211::SNR (double signal, double noiseInterference, TransmissionMode *mode)
{
#ifdef PHY80211_DEBUG
	cout << "S: " << signal << 
		", Ni: " << noiseInterference << 
		", Nf: " << calculateNoiseFloor (mode->getSignalSpread ()) << endl;
#endif
	double noise = calculateNoiseFloor (mode->getSignalSpread ()) + noiseInterference;
	double snr = signal / noise;
	return snr;
}

double
Phy80211::calculateCurrentNoiseInterference (void)
{
	double noiseInterference;
	noiseInterference = calculateNoiseInterference (now ());
	return noiseInterference;
}

/* return power in W */
double 
Phy80211::calculatePower (Packet *p)
{
	double power;
	PacketStamp stamp;
	stamp.stamp(peekMobileNode (), m_antenna, 0, getLambda ());
	power = m_propagation->Pr (&p->txinfo_, &stamp, getSystemLoss (), getLambda ());
	return power;
}


double
Phy80211::calculateNoiseInterference (double time)
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

void
Phy80211::startTx (Packet *packet)
{
	/* Transmission can happen if:
	 *  - we are syncing on a packet. It is the responsability of the
	 *    MAC layer to avoid doing this but the PHY does nothing to 
	 *    prevent it.
	 *  - we are idle
	 */
	assert (getState () != Phy80211::SLEEP &&
		getState () != Phy80211::TX);

	if (getState () == Phy80211::SYNC) {
		m_endRxHandler->cancel ();
	}

	/* txPower should be calculated on a per-packet basis by the MAC.
	 * It is actually a matter of:
	 *    - getting the MAC to set this field.
	 *    - leave this field alone here.
	 * XXX
	 */
	MobileNode *node = peekMobileNode ();
	Antenna *antenna = m_antenna->copy();
	packet->txinfo_.stamp(node, antenna, dBmToW (m_txPower), getLambda ());

	double txDuration = calculatePacketDuration (getHeaderMode (packet), 
						     getPayloadMode (packet),
						     getSize (packet));
	notifyTxStart (now (), txDuration);
	switchToTx (txDuration);
	peekChannel ()->recv (packet, this);
}

void
Phy80211::startRx (Packet *packet)
{
#ifdef PHY80211_DEBUG
	cout << this << " phy startRx " << now () << " " << packet << endl;
#endif
	PhyRxEvent *event = new PhyRxEvent (packet, this);
	appendEvent (event);

	switch (getState ()) {
	default:
		cout << "FDASFASDFASDF" << endl;
		break;
	case Phy80211::SYNC:
	case Phy80211::TX:
	case Phy80211::SLEEP:
		Packet::free (packet);
		break;
	case Phy80211::IDLE: {
		double power = calculatePower (packet);
		
		if (power > dBmToW (m_rxThreshold)) {
			// sync to signal
			notifyRxStart (now (), event->getDuration ());
			switchToSyncFromIdle ();
			m_endRxHandler->start (event, packet, 
					       event->getDuration ());
		} else {
#ifdef PHY80211_DEBUG
			cout << power << "too small " << dBmToW (m_rxThreshold) << endl;
#endif
			/* if the energy of the signal is smaller than rxThreshold,
			 * this packet is not synced upon.
			 */
			Packet::free (packet);
		}
	} break;
	}
	event->unref ();
}

double
Phy80211::calculateNI (PhyRxEvent *phyRxEvent, vector <NIChange> *ni)
{
	list<PhyRxEvent *>::iterator i = m_rxEventList.begin ();
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
	sort (ni->begin (), ni->end (), less<NIChange> ());

#ifdef PHY80211_DEBUG
	{
		cout << "snir: " << endl;
		vector<NIChange>::iterator j;
		for (j = ni->begin (); j < ni->end (); j++) {
			cout << (*j).getTime () << " ";
			cout << (*j).getDelta () << " ";
			cout << endl;
		}
	}
#endif 

	return noiseInterference;
}

TransmissionMode *
Phy80211::getMode (int mode)
{
	return m_modes[mode];
}

double
Phy80211::calculateChunkSuccessRate (double snir, double delay, TransmissionMode *mode)
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
Phy80211::calculatePER (PhyRxEvent *packet, vector <NIChange> *ni)
{	
	double psr = 1; /* Packet Success Rate */
	vector<NIChange>::iterator j = ni->begin ();
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

#ifdef PHY80211_DEBUG
	cout << "psr: " << psr << endl;
#endif
	double per = 1 - psr;
	return per;
}

void 
Phy80211::endRx (PhyRxEvent *phyRxEvent, Packet *packet)
{
#ifdef PHY80211_DEBUG
	cout << "phy endRx " << now () << endl;
#endif

	vector<NIChange> ni;

	assert (getState () == Phy80211::SYNC);

	assert (phyRxEvent->getEndTime () == 
		Scheduler::instance ().clock ());

	/* calculate the SNIR at the start of the packet and accumulate
	 * all SNIR changes in the snir vector.
	 */
	// XXX: we assign NI to SNR. Wrong !!
	m_rxStartSNR = calculateNI (phyRxEvent, &ni);
	m_rxStartTime = phyRxEvent->getStartTime ();

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
	uptarget ()->recv (packet);
}

void
Phy80211::switchToTx (double txDuration)
{
	assert (m_endTx == 0);
	switch (getState ()) {
	case Phy80211::SYNC:
		/* If we were receiving a packet when this tx
		 * started, we drop it now. It will be discarded 
		 * later in endRx.
		 */
		assert (!m_rxing);
		m_rxing = false;
		STATE_FROM (Phy80211::SYNC);
		break;
	case Phy80211::IDLE:
		STATE_FROM (Phy80211::IDLE);
		break;
	default:
		assert (false);
		break;
	}
	m_previousStateChangeTime = now ();
	m_endTx = now () + txDuration;
	STATE_TO (Phy80211::TX);
	STATE_AT (now ());
}
void
Phy80211::switchToSyncFromIdle (void)
{
	assert (getState () == Phy80211::IDLE);
	assert (!m_rxing);
	m_previousStateChangeTime = now ();
	m_rxing = true;
	assert (getState () == Phy80211::SYNC);
	STATE_FROM (Phy80211::IDLE);
	STATE_TO (Phy80211::SYNC);
	STATE_AT (now ());
}
void
Phy80211::switchToSleep (void)
{
	assert (!m_sleeping);
	switch (getState ()) {
	case Phy80211::SYNC:
		/* If we were receiving a packet when this sleep is
		 * started, we drop it now. It will be discarded 
		 * later in endRx.
		 */
		assert (m_rxing);
		m_rxing = false;
		STATE_FROM (Phy80211::SYNC);
		break;
	case Phy80211::IDLE:
		/* */
		STATE_FROM (Phy80211::IDLE);
		break;
	case Phy80211::TX:
		/* If we were transmitting a packet when this sleep
		 * started, we cannot drop it as we should (obviously,
		 * the transmission will not be able to complete)
		 * because the packet has already been put in the 
		 * reception queue of all the target nodes. To be
		 * able to drop it, we would need to remove it from
		 * each target queue or notify each target to remove 
		 * it.
		 * I know, this sucks and it is a bug but there is no
		 * reasonable fix to it.
		 */
		assert (false);
		break;
	default:
		assert (false);
	}
	m_previousStateChangeTime = now ();
	m_sleeping = true;
	STATE_FROM (Phy80211::SLEEP);
	STATE_AT (now ());
}
void
Phy80211::switchToIdleFromSleep (void)
{
	assert (getState () == Phy80211::SLEEP);
	assert (!m_sleeping);

	m_previousStateChangeTime = now ();
	m_sleeping = false;

	assert (getState () == Phy80211::IDLE);

	STATE_FROM (Phy80211::SLEEP);
	STATE_TO (Phy80211::IDLE);
	STATE_AT (now ());
}
void
Phy80211::switchToIdleFromSync (void)
{
	assert (getState () == Phy80211::SYNC);
	assert (m_rxing);

	m_previousStateChangeTime = now ();
	m_rxing = false;

	assert (getState () == Phy80211::IDLE);

	STATE_FROM (Phy80211::SYNC);
	STATE_TO (Phy80211::IDLE);
	STATE_AT (now ());
}
enum Phy80211::Phy80211State 
Phy80211::getState (void)
{
	if (m_sleeping) {
		assert (m_endTx == 0);
		assert (!m_rxing);
		return Phy80211::SLEEP;
	} else {
		if (m_endTx != 0 && m_endTx > now ()) {
			return Phy80211::TX;
		} else if (m_endTx != 0) {
			/* At one point in the past, we completed
			 * transmission of this packet.
			 */
			STATE_FROM (Phy80211::TX);
			STATE_TO (Phy80211::IDLE);
			STATE_AT (m_endTx);
			
			m_previousStateChangeTime = m_endTx;
			m_endTx = 0;
		}
		if (m_rxing) {
			return Phy80211::SYNC;
		} else {
			return Phy80211::IDLE;
		}
	}
}
double
Phy80211::getStateDuration (void)
{
	return now () - m_previousStateChangeTime;
}
double 
Phy80211::getDelayUntilIdle (void)
{
	return 0.0;
}

Channel *
Phy80211::peekChannel (void)
{
	return Phy::channel ();
}

MobileNode *
Phy80211::peekMobileNode (void)
{
	return static_cast<MobileNode *>(Phy::node ());
}

void 
Phy80211::appendEvent (PhyRxEvent *event)
{
	/* attempt to remove the events which are 
	 * not useful anymore. 
	 * i.e.: all events which end _before_
	 *       now - m_maxPacketDuration
	 */
	double end = now () - m_maxPacketDuration;
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
