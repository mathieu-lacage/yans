/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "simulator.h"
#include "arp.h"
#include "arp-cache-entry.h"

ArpCacheEntry::ArpCacheEntry (Arp *arp)
	: m_arp (arp),
	  m_state (ALIVE)
{}


bool 
ArpCacheEntry::is_dead (void)
{
	return (m_state == DEAD)?true:false;
}
bool 
ArpCacheEntry::is_alive (void)
{
	return (m_state == ALIVE)?true:false;
}
bool
ArpCacheEntry::is_wait_reply (void)
{
	return (m_state == WAIT_REPLY)?true:false;
}


void 
ArpCacheEntry::mark_dead (void) 
{
	m_state = DEAD;
	update_seen ();
}
void 
ArpCacheEntry::mark_alive (int mac_address) 
{
	m_u.m_mac_address = mac_address;
	m_state = ALIVE;
	update_seen ();
}

Packet *
ArpCacheEntry::update_wait_reply (Packet *waiting)
{
	assert (m_state == WAIT_REPLY);
	/* We are already waiting for an answer so
	 * we dump the previously waiting packet and
	 * replace it with this one.
	 */
	Packet *old = m_u.m_waiting;
	m_u.m_waiting = waiting;
	return old;
}
void 
ArpCacheEntry::mark_wait_reply (Packet *waiting)
{
	assert (m_state == ALIVE || m_state == DEAD);
	m_state = WAIT_REPLY;
	m_u.m_waiting = waiting;
	update_seen ();
}

Packet *
ArpCacheEntry::get_waiting_packet (void)
{
	assert (m_state == WAIT_REPLY);
	return m_u.m_waiting;
}

int 
ArpCacheEntry::get_mac_address (void)
{
	assert (m_state == ALIVE);
	return m_u.m_mac_address;
}
bool 
ArpCacheEntry::is_expired (void)
{
	double timeout;
	switch (m_state) {
	case ArpCacheEntry::WAIT_REPLY:
		timeout = m_arp->get_wait_reply_timeout ();
		break;
	case ArpCacheEntry::DEAD:
		timeout = m_arp->get_dead_timeout ();
		break;
	case ArpCacheEntry::ALIVE:
		timeout = m_arp->get_alive_timeout ();
		break;
	}
	double elapsed_since_last_event = now () - m_last_seen_time;
	if (elapsed_since_last_event >= timeout) {
		return true;
	} else {
		return false;
	}
}
void 
ArpCacheEntry::update_seen (void)
{
	m_last_seen_time = now();
}
double 
ArpCacheEntry::now (void)
{
	return Simulator::instance ()->now_s ();
}
