/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "packet.h"
#include "chunk.h"

PacketDestroyNotifier::~PacketDestroyNotifier ()
{}

Packet::Packet ()
	: m_ref (1)
{}

Packet::~Packet ()
{
	for (PacketDestroyNotifiersCI k = m_destroy_notifiers.begin ();
	     k != m_destroy_notifiers.end (); k++) {
		(*k)->notify (this);
		delete (*k);
	}
	m_destroy_notifiers.erase (m_destroy_notifiers.begin (),
				   m_destroy_notifiers.end ());
	for (HeadersCI i = m_headers.begin (); i != m_headers.end (); i++) {
		delete (*i);
	}
	m_headers.erase (m_headers.begin (), m_headers.end ());
	for (TrailersCI j = m_trailers.begin (); j != m_trailers.end (); j++) {
		delete (*j);
	}
	m_trailers.erase (m_trailers.begin (), m_trailers.end ());
}

void 
Packet::add_destroy_notifier (PacketDestroyNotifier *notifier)
{
	m_destroy_notifiers.push_back (notifier);
}

void 
Packet::ref (void)
{
	m_ref++;
}

void 
Packet::unref (void)
{
	m_ref--;
	if (m_ref == 0) {
		delete this;
	}
}

void 
Packet::add_tag (uint32_t tag_id, Tag *tag)
{
	assert (m_tags.find (tag_id) == m_tags.end ());
	m_tags[tag_id] = tag;
}
Tag *
Packet::get_tag (uint32_t tag_id)
{
	if (m_tags.find (tag_id) == m_tags.end ()) {
		return 0;
	} else {
		return m_tags[tag_id];
	}
}
Tag *
Packet::remove_tag (uint32_t tag_id)
{
	if (m_tags.find (tag_id) == m_tags.end ()) {
		return 0;
	} else {
		Tag *tag = m_tags[tag_id];
		m_tags.erase (m_tags.find (tag_id));
		return tag;
	}
}


void 
Packet::add_header (Chunk *header)
{
	m_headers.push_back (header);
}
void 
Packet::add_trailer (Chunk *trailer)
{
	m_trailers.push_back (trailer);
}
Chunk *
Packet::remove_header (void)
{
	Chunk *header = m_headers.back ();
	m_headers.pop_back ();
	return header;
}
Chunk *
Packet::remove_trailer (void)
{
	Chunk *trailer = m_trailers.back ();
	m_trailers.pop_back ();
	return trailer;
}
Chunk const *
Packet::peek_header (void)
{
	return m_headers.back ();
}
Chunk const *
Packet::peek_trailer (void)
{
	return m_trailers.back ();
}

uint32_t 
Packet::get_size (void)
{
	uint32_t size = 0;
	for (HeadersCI i = m_headers.begin (); i != m_headers.end (); i++) {
		size += (*i)->get_size ();
	}
	for (TrailersCI j = m_trailers.begin (); j != m_trailers.end (); j++) {
		size += (*j)->get_size ();
	}
	return size;
}

void 
Packet::serialize (WriteBuffer *buffer)
{
	for (HeadersRI i = m_headers.rbegin (); i != m_headers.rend (); i++) {
		(*i)->serialize (buffer);
	}
	for (TrailersCI j = m_trailers.begin (); j != m_trailers.end (); j++) {
		(*j)->serialize (buffer);
	}
}

void 
Packet::print (std::ostream *os)
{
	Headers::size_type k = 0;
	for (HeadersRI i = m_headers.rbegin (); i != m_headers.rend (); i++) {
		(*i)->print (os);
		k++;
		if (k < m_headers.size ()) {
			*os << " | ";
		}
	}
	Trailers::size_type l = 0;
	for (TrailersCI j = m_trailers.begin (); j != m_trailers.end (); j++) {
		(*j)->print (os);
		l++;
		if (l < m_trailers.size ()) {
			*os << " | ";
		}
	}
}

Packet *
Packet::copy (void)
{
	Packet *other = new Packet ();
	for (HeadersCI i = m_headers.begin (); i != m_headers.end (); i++) {
		Chunk *new_chunk = (*i)->copy ();
		other->add_header (new_chunk);
	}
	for (TrailersCI j = m_trailers.begin (); j != m_trailers.end (); j++) {
		Chunk *new_chunk = (*j)->copy ();
		other->add_trailer (new_chunk);
	}
	return other;
}
