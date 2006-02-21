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

#ifndef PHY_80211_H
#define PHY_80211_H

#include <stdint.h>
#include <vector>
#include <stdint.h>
#include "callback.tcc"
#include "propagation-model.h"

namespace yans {

class TransmissionMode;
class Packet;

class Phy80211Listener {
public:
	virtual ~Phy80211Listener ();

	/* we have received the first bit of a packet. We decided
	 * that we could synchronize on this packet. It does not mean
	 * we will be able to successfully receive completely the
	 * whole packet. It means we will report a BUSY status.
	 * rxEnd will be invoked later to report whether or not
	 * the packet was successfully received.
	 */
	virtual void notify_rx_start (double now, double duration) = 0;
	/* we have received the last bit of a packet for which
	 * rxStart was invoked first. 
	 */
	virtual void notify_rx_end (double now, bool received_ok) = 0;
	/* we start the transmission of a packet.
	 */
	virtual void notify_tx_start (double now, double duration) = 0;
	virtual void notify_sleep (double now) = 0;
	virtual void notify_wakeup (double now) = 0;
};



class Phy80211
{
public:
	typedef Callback<void (Packet *, PropagationData const &,int tx_mode)> TxCallback;
	typedef Callback<void (Packet *, double, int)> RxOkCallback;
	typedef Callback<void (Packet *)> RxErrorCallback;

	Phy80211 ();
	virtual ~Phy80211 ();

	void set_tx_callback (TxCallback *callback);
	void set_receive_ok_callback (RxOkCallback *callback);
	void set_receive_error_callback (RxErrorCallback *callback);

	/* packet was sent from [x,y,z] with tx_mode 
	 * at power tx_power (in dBm)
	 */
	void receive_packet (Packet *packet, 
			     PropagationData const &data,
			     int tx_mode);
	void send_packet (Packet *packet, int tx_mode, int tx_power);

	void sleep (void);
	void wakeup (void);

	void register_listener (Phy80211Listener *listener);

	bool is_state_idle (void);
	bool is_state_busy (void);
	bool is_state_rx (void);
	bool is_state_tx (void);
	bool is_state_sleep (void);
	double get_state_duration (void);
	double get_delay_until_idle (void);

	double calculate_tx_duration (Packet *packet);
	double calculate_tx_duration (int payload_mode, int size);

	void configure_80211a (void);
	/* */
	void set_ed_threshold (double rx_threshold);
	void set_rx_noise (double rx_noise);	
	/* tx_power_{base|end} are dBm units */
	void set_tx_power_increments (double tx_power_base, 
				      double tx_power_end, 
				      int n_tx_power);
	uint32_t get_n_modes (void);
	uint32_t get_n_txpower (void);

	void set_propagation_model (PropagationModel propagation);

private:
	enum Phy80211State {
		SYNC,
		TX,
		IDLE,
		SLEEP
	};
	enum Phy80211State get_state (void);

	virtual void startRx (Packet *packet) = 0;
	virtual void cancelRx (void) = 0;
protected:
	void notifyRxStart (double now, double duration);
	void notifyRxEnd (double now, bool receivedOk);
	void switchToSyncFromIdle (double rxDuration);
	void switchToIdleFromSync (void);
	double now (void);
	double calculateRxPower (Packet *p);
	double dBmToW (double dBm);
	double getRxThreshold (void);
	int    getPayloadMode (Packet *packet);
	double getMaxPacketDuration (void);
	double calculatePacketDuration (int headerMode, int payloadMode, int size);
	double getPreambleDuration (void);
	double calculateHeaderDuration (int headerMode);
	TransmissionMode *getMode (int mode);
	void setLastRxSNR (double snr);
	int selfAddress (void);
	double SNR (double signal, double noiseInterference, TransmissionMode *mode);
	void forwardUp (Packet *packet);
private:
	double       m_frequency;
	double       m_plcpPreambleDelay;
	uint32_t     m_plcpHeaderLength;

	double       m_systemLoss;
	double       m_ed_threshold;
	double       m_rx_noise;
	double       m_tx_power_base;
	double       m_tx_power_end;
	uint32_t     m_n_txpower;
	double       m_maxPacketDuration;

	void switchToTx (double txDuration);
	void switchToSleep (void);
	void switchToIdleFromSleep (void);
	
	void addTxRxMode (TransmissionMode *mode);

	double calculateNoiseFloor (double signalSpread);
	int    getHeaderMode (Packet *packet);
	double dBToRatio (double dB);
	double max (double a, double b);
	char const *stateToString (enum Phy80211State state);

	void notifyTxStart (double now, double duration);
	void notifySleep (double now);
	void notifyWakeup (double now);

	PropagationModel m_propagation;
	typedef std::vector<Phy80211Listener *> Listeners;
	typedef std::vector<Phy80211Listener *>::iterator ListenersCI;
	typedef std::vector<class TransmissionMode *> Modes;
	typedef std::vector<class TransmissionMode *>::iterator ModesI;
	Listeners m_listeners;
	Modes m_modes;

	double m_rxStartSNR;
	bool m_sleeping;
	bool m_rxing;
	double m_end_tx;
	double m_end_rx;
	double m_previous_state_change_time;

	TxCallback *m_tx_callback;
	RxOkCallback *m_rx_ok_callback;
	RxErrorCallback *m_rx_error_callback;
};

}; // namespace yans


#endif /* PHY_80211_H */
