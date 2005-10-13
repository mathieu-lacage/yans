/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef ARP_CACHE_ENTRY_H
#define ARP_CACHE_ENTRY_H

class ArpCacheEntry {
public:
	ArpCacheEntry (Arp *arp);

	void mark_dead (void);
	void mark_alive (int mac_address);
	void mark_wait_reply (Packet *waiting);
	Packet *update_wait_reply (Packet *waiting);

	bool is_dead (void);
	bool is_alive (void);
	bool is_wait_reply (void);

	Packet *get_waiting_packet (void);
	int get_mac_address (void);
	bool is_expired (void);
private:
	enum ArpCacheEntryState_e {
		ALIVE,
		WAIT_REPLY,
		DEAD
	};

	void update_seen (void);
	double now (void);
	ArpCacheEntryState_e get_state (void);
	Arp *m_arp;
	ArpCacheEntryState_e m_state;
	double m_last_seen_time;
	union {
		MacAddress m_mac_address;
		Packet *m_waiting;
	} m_u;
};

#endif /* ARP_CACHE_ENTRY_H */
