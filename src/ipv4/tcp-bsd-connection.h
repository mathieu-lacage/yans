/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef TCP_BSD_CONNECTION_H
#define TCP_BSD_CONNECTION_H

#include "callback.h"
#include "ipv4-address.h"
#include <stdint.h>
#include "tcp-bsd-timers.h"

class Packet;
class Ipv4;
class Host;
class TcpEndPoint;
class Route;
class TcpPieces;
class ChunkTcp;


class TcpBsdConnection {
public:
	typedef Callback<void (void)> ConnectCompletedCallback;
	typedef Callback<void (void)> DisConnectRequestedCallback;
	typedef Callback<void (void)> DisConnectCompletedCallback;
	typedef Callback<void (void)> DataReceivedCallback;
	typedef Callback<void (void)> DataTransmittedCallback;
	typedef Callback<void (Packet *)> AckReceivedCallback;
	typedef Callback<void (TcpBsdConnection *)> TcpBsdConnectionDestroy;

	TcpBsdConnection ();
	~TcpBsdConnection ();

	void set_ipv4 (Ipv4 *ipv4);
	void set_host (Host *host);
	void set_end_point (TcpEndPoint *end_point);
	void set_route (Route *route);
	void set_destroy_handler (TcpBsdConnectionDestroy *handler);

	void set_callbacks (ConnectCompletedCallback *connect_completed,
			    DisConnectRequestedCallback *disconnect_requested,
			    DisConnectCompletedCallback *disconnect_completed,
			    DataTransmittedCallback *data_transmitted,
			    DataReceivedCallback *data_received,
			    AckReceivedCallback *ack_received);
	void start_connect (void);
	void start_disconnect (void);

	uint32_t get_room_left (void);
	uint32_t get_data_ready (void);
	uint32_t send (Packet *packet);
	Packet *recv (uint32_t size);

	void slow_timer (void);
	void fast_timer (void);
private:
	typedef unsigned char u_char;
	typedef unsigned short u_short;
	typedef unsigned int u_int;
	typedef unsigned long u_long;
	typedef	uint32_t tcp_seq;
	typedef uint32_t tcp_cc;		/* connection count per rfc1644 */
#ifndef NULL
	enum { NULL = 0 };
#endif
	typedef uint8_t u_int8_t;
	typedef uint16_t u_int16_t;
	typedef uint32_t u_int32_t;


	void trigger_events (void);
	void set_state (int new_state);
	void respond(tcp_seq ack, tcp_seq seq, int flags);
	void timers (int timer);
	void input (Packet *packet);
	int output (void);
	void drop (int errno);
	void xmit_timer (short rtt);
	void canceltimers (void);
	u_int mss (u_int offer);
	void dooptions (ChunkTcp *tcp, int *ts_present, u_long *ts_val, u_long *ts_ecr);
	void setpersist (void);
	int reass(ChunkTcp *tcp, Packet *packet);

	TcpEndPoint *m_end_point;
	Route *m_route;
	ConnectCompletedCallback *m_connect_completed;
	DisConnectCompletedCallback *m_disconnect_completed;
	DisConnectRequestedCallback *m_disconnect_requested;
	DataReceivedCallback *m_data_received;
	DataTransmittedCallback *m_data_transmitted;
	AckReceivedCallback *m_ack_received;
	TcpBsdConnectionDestroy *m_destroy;
	Ipv4 *m_ipv4;
	Host *m_host;
	TcpPieces *m_send;
	TcpPieces *m_recv;


	short	m_t_state;		/* state of this connection */
	short	m_t_timer[TCPT_NTIMERS];	/* tcp timers */
	short	m_t_rxtshift;		/* log(2) of rexmt exp. backoff */
	short	m_t_rxtcur;		/* current retransmit value */
	short	m_t_dupacks;		/* consecutive dup acks recd */
	u_short	m_t_maxseg;		/* maximum segment size */
	char	m_t_force;		/* 1 if forcing out a byte */
	u_short	m_t_flags;
#define	TF_ACKNOW	0x0001		/* ack peer immediately */
#define	TF_DELACK	0x0002		/* ack, but try to delay it */
#define	TF_NODELAY	0x0004		/* don't delay packets to coalesce */
#define	TF_NOOPT	0x0008		/* don't use tcp options */
#define	TF_SENTFIN	0x0010		/* have sent FIN */
#define	TF_REQ_SCALE	0x0020		/* have/will request window scaling */
#define	TF_RCVD_SCALE	0x0040		/* other side has requested scaling */
#define	TF_REQ_TSTMP	0x0080		/* have/will request timestamps */
#define	TF_RCVD_TSTMP	0x0100		/* a timestamp was received in SYN */
#define	TF_SACK_PERMIT	0x0200		/* other side said I could SACK */

/*
 * The following fields are used as in the protocol specification.
 * See RFC783, Dec. 1981, page 21.
 */
/* send sequence variables */
	tcp_seq	m_snd_una;		/* send unacknowledged */
	tcp_seq	m_snd_nxt;		/* send next */
	tcp_seq	m_snd_up;			/* send urgent pointer */
	tcp_seq	m_snd_wl1;		/* window update seg seq number */
	tcp_seq	m_snd_wl2;		/* window update seg ack number */
	tcp_seq	m_iss;			/* initial send sequence number */
	u_long	m_snd_wnd;		/* send window */
/* receive sequence variables */
	u_long	m_rcv_wnd;		/* receive window */
	tcp_seq	m_rcv_nxt;		/* receive next */
	tcp_seq	m_rcv_up;			/* receive urgent pointer */
	tcp_seq	m_irs;			/* initial receive sequence number */
/*
 * Additional variables for this implementation.
 */
/* receive variables */
	tcp_seq	m_rcv_adv;		/* advertised window */
/* retransmit variables */
	tcp_seq	m_snd_max;		/* highest sequence number sent;
					 * used to recognize retransmits
					 */
/* congestion control (for slow start, source quench, retransmit after loss) */
	u_long	m_snd_cwnd;		/* congestion-controlled window */
	u_long	m_snd_ssthresh;		/* snd_cwnd size threshhold for
					 * for slow start exponential to
					 * linear switch
					 */
/*
 * transmit timing stuff.  See below for scale of srtt and rttvar.
 * "Variance" is actually smoothed difference.
 */
	short	m_t_idle;			/* inactivity time */
	short	m_t_rtt;			/* round trip time */
	tcp_seq	m_t_rtseq;		/* sequence number being timed */
	short	m_t_srtt;			/* smoothed round-trip time */
	short	m_t_rttvar;		/* variance in round-trip time */
	u_short	m_t_rttmin;		/* minimum rtt allowed */
	u_long	m_max_sndwnd;		/* largest window peer has offered */

/* out-of-band data */
	char	m_t_oobflags;		/* have some */
	char	m_t_iobc;			/* input character */
#define	TCPOOB_HAVEDATA	0x01
#define	TCPOOB_HADDATA	0x02
	short	m_t_softerror;		/* possible error not yet reported */

/* RFC 1323 variables */
	u_char	m_snd_scale;		/* window scaling for send window */
	u_char	m_rcv_scale;		/* window scaling for recv window */
	u_char	m_request_r_scale;	/* pending window scaling */
	u_char	m_requested_s_scale;
	u_long	m_ts_recent;		/* timestamp echo data */
	u_long	m_ts_recent_age;		/* when last updated */
	tcp_seq	m_last_ack_sent;


	int m_tcp_keepidle;              /* time before keepalive probes begin */
	int m_tcp_keepintvl;             /* time between keepalive probes */
	int m_tcp_maxidle;               /* time to drop after starting probes */
	int m_tcp_ttl;                   /* time to live for TCP segs */
	int m_tcp_backoff[1];


#define ETIMEDOUT 10
#define ECONNREFUSED 11
#define ECONNRESET 12
	int m_so_error;
	int m_so_options;
};


#endif /* TCP_BSD_CONNECTION_H */
