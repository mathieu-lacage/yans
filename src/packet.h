/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <ostream>
#include <vector>
#include "sgi-hashmap.h"

#include "tag-manager.h"

class Chunk;
class WriteBuffer;
class ReadBuffer;
class Packet;

class PacketDestroyNotifier {
public:
	virtual ~PacketDestroyNotifier ();
	virtual void notify (Packet *packet) = 0;
};

class Packet {
public:
	Packet ();
	~Packet ();

	void ref (void);
	void unref (void);

	void add_destroy_notifier (PacketDestroyNotifier *notifier);

	void add_tag (uint32_t tag_id, Tag *tag);
	Tag *get_tag (uint32_t tag_id);
	Tag *remove_tag (uint32_t tag_id);

	void add_header (Chunk *header);
	void add_trailer (Chunk *trailer);
	Chunk *remove_header (void);
	Chunk *remove_trailer (void);
	Chunk const *peek_header (void);
	Chunk const *peek_trailer (void);

	uint32_t get_size (void);

	void serialize (WriteBuffer *buffer);

	void print (std::ostream *os);

	Packet *copy (void);
 private:
	typedef std::vector<Chunk *> Headers;
	typedef std::vector<Chunk *>::const_iterator HeadersCI;
	typedef std::vector<Chunk *>::reverse_iterator HeadersRI;
	typedef std::vector<Chunk *> Trailers;
	typedef std::vector<Chunk *>::const_iterator TrailersCI;
	typedef std::vector<PacketDestroyNotifier *> PacketDestroyNotifiers;
	typedef std::vector<PacketDestroyNotifier *>::const_iterator PacketDestroyNotifiersCI;
	typedef Sgi::hash_map<uint32_t, Tag *> Tags;
	PacketDestroyNotifiers m_destroy_notifiers;
	Headers m_headers;
	Trailers m_trailers;
	Tags m_tags;
	uint32_t m_ref;
};

#endif /* PACKET_H */
