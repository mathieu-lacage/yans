/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "packet.h"
#include "chunk.h"

Packet::Packet ()
	: m_ref (1)
{}

Packet::~Packet ()
{
	for (HeadersI i = m_headers.begin (); i != m_headers.end (); i = m_headers.erase (i)) {
		delete (*i);
	}
	for (TrailersI j = m_trailers.begin (); j != m_trailers.end (); j = m_trailers.erase (j)) {
		delete (*j);
	}
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
	for (HeadersRI i = m_headers.rbegin (); i != m_headers.rend (); i++) {
		(*i)->print (os);
	}
	for (TrailersCI j = m_trailers.begin (); j != m_trailers.end (); j++) {
		(*j)->print (os);
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
