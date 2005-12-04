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

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "callback.h"
#include "ipv4-address.h"
#include <stdint.h>

class Packet;
class Ipv4;
class Host;
class TcpEndPoint;
class Route;
class TcpPieces;


class TcpConnection {
public:
	typedef Callback<void (void)> ConnectionCompletedCallback;
	typedef Callback<void (void)> DataReceivedCallback;
	typedef Callback<void (Packet *)> AckReceivedCallback;
	typedef Callback<void (TcpConnection *)> TcpConnectionDestroy;

	TcpConnection ();
	~TcpConnection ();

	void set_ipv4 (Ipv4 *ipv4);
	void set_host (Host *host);
	void set_end_point (TcpEndPoint *end_point);
	void set_route (Route *route);
	void set_destroy_handler (TcpConnectionDestroy *handler);

	void set_callbacks (ConnectionCompletedCallback *connection_completed,
			    DataReceivedCallback *data_received,
			    AckReceivedCallback *ack_received);
	void start_connect (void);

	uint32_t get_room_left (void);
	uint32_t get_data_ready (void);
	uint32_t send (Packet *packet);
	Packet *recv (uint32_t size);

	void slow_timer (void);
	void fast_timer (void);
private:
	enum TcpState_e {
		CLOSED,
		LISTEN,
		SYN_SENT,
		SYN_RCVD,
		ESTABLISHED,
		CLOSE_WAIT,
		LAST_ACK,
		FIN_WAIT_1,
		CLOSING,
		TIME_WAIT,
		FIN_WAIT_2
	};
	void receive (Packet *packet);
	void set_state (enum TcpState_e new_state);
	bool invert_packet (Packet *packet);
	uint32_t get_isn (void);
	bool send_syn_ack (Packet *packet);
	bool send_ack (Packet *packet);
	void send_out (Packet *packet);
	void retransmission_timeout (void);
	void start_retransmission_timer (void);
	void notify_data_ready_to_send (void);
	void notify_room_ready_to_receive (void);
	void send_data (void);

	TcpEndPoint *m_end_point;
	Route *m_route;
	ConnectionCompletedCallback *m_connection_completed;
	DataReceivedCallback *m_data_received;
	AckReceivedCallback *m_ack_received;
	enum TcpState_e m_state;
	Ipv4 *m_ipv4;
	Host *m_host;
	TcpConnectionDestroy *m_destroy;

	TcpPieces *m_send;
	TcpPieces *m_recv;

	/* The variables below are best explained with the rfc 793 diagrams
	 * reproduced below for the sake of clarity:
	 *                   1         2          3          4      
	 *              ----------|----------|----------|---------- 
	 *                     SND.UNA    SND.NXT    SND.UNA        
	 *                                          +SND.WND        
	 *
	 *        1 - old sequence numbers which have been acknowledged  
	 *        2 - sequence numbers of unacknowledged data            
	 *        3 - sequence numbers allowed for new data transmission 
	 *        4 - future sequence numbers which are not yet allowed  
	 *
	 *                        1          2          3      
	 *                   ----------|----------|---------- 
	 *                          RCV.NXT    RCV.NXT        
	 *                                    +RCV.WND        
	 *
	 *        1 - old sequence numbers which have been acknowledged  
	 *        2 - sequence numbers allowed for new reception         
	 *        3 - future sequence numbers which are not yet allowed  
	 */
	uint32_t m_snd_una;
	uint32_t m_snd_nxt;
	uint32_t m_snd_wnd;
	uint32_t m_rcv_nxt;
	uint32_t m_rcv_wnd;
	/* We have also these:
	 * SND.WL1 - segment sequence number used for last window update
	 * SND.WL2 - segment acknowledgment number used for last window
	 *           update
	 */
	uint32_t m_snd_wl1;
	uint32_t m_snd_wl2;
	/* The following variables are also needed although not described
	 * in the tcp rfc. I have choosen to use the same names as the BSD
	 * implementation for the sake of clarity.
	 */
	uint32_t m_rcv_adv;  /* receive window advertised by other end. */
	uint32_t m_snd_cwnd; /* congestion window. */
	uint32_t m_snd_ssthresh; /* snd_cwnd threshold for slow start. */
	uint32_t m_snd_max; /* the largest sequence number of packets sent. */
	uint32_t m_max_sndwnd; /* largest window ever offered by other end. */

	uint32_t m_snd_mss; /* maximum segment size to send */


	uint32_t m_retransmission_timer;

	/* tcp receive and transmission buffers where packets to send 
	 * and receive are accumulated.
	 */
	
};


#endif /* TCP_CONNECTION_H */
