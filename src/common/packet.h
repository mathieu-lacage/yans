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

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <ostream>
#include <list>
#include <utility>

#include "callback.h"
#include "gtags.h"
#include "ref-ptr.h"

namespace yans {

class Packet;
class Chunk;
class Buffer;

/**
 * \brief smart-pointer for Packet
 *
 * PacketPtr objects are pointers to Packet instances. They
 * behave like any normal pointer: you can assign 0 to them,
 * compare them against 0 and pass them around by value.
 * They manage the ressources of the Packet they point to
 * automatically.
 */
typedef RefPtr<Packet> PacketPtr;
/**
 * \brief smart-pointer for const Packet
 *
 * ConstPacketPtr objects are pointers to Packet instances,
 * just like PacketPtr, except that they point to instances
 * of _const_ Packets which means you cannot invoke any 
 * non-const method on the underlying Packet.
 */
typedef RefPtr<Packet const> ConstPacketPtr;

/**
 * \brief network Packet
 *
 * Network Packets are represented by an array of bytes. Each simulation
 * byte is expected have a one-to-one correspondance to a simulated byte.
 * Packets are always accessed though the PacketPtr or ConstPacketPtr classes
 * which are pointers to Packets. 
 * It is also possible to attach a single instance of any number of Tag types
 * to each Packet: this makes it easy to attach per-packet data to exchange 
 * information across multiple network layers. 
 *
 *  - Packets are created with the Packet::create method.
 *  - Packets are fragmented with the Packet::copy methods
 *  - Packets are re-assembled with the Packet::add_at_end methods
 *  - protocol headers (or trailers) are prepended (or appended) 
 *    to Packets with the Packet::add method
 *  - protocol headers (or trailers) are removed from Packets with
 *    the Packet::remove method
 *  - Data can be trimmed from a Packet with the Packet::remove_at_start
 *    and Packet::remove_at_end methods
 *  - Tags can be attached and dettached from a Packet with the
 *    Packet::add_tag and Packet::remove_tag methods
 *
 * Each protocol header must be represented by a class which derives from
 * the abstract base class Chunk. It must implement the Chunk::add_to and
 * Chunk::remove_from methods and these methods are responsible for serializing
 * and deserializing the protocol header data to and from a byte stream.
 * The serialized representation should be:
 *   - portable
 *   - as close as possible to the serialized version of real network packets.
 *
 * Experiments show that generating the _exact_ bit-by-bit layout
 * of a TCP or IPv4 header is at most 10% slower than doing a quick and dirty 
 * dump of the data through a memcpy.
 *
 * Each tag must be represented by a class which derives from the abstract
 * base class Tag. It must implement the Tag::real_get_id and Tag::real_get_size
 * methods to allow the Packet class to track the type of each tag and 
 * perform tag copies.
 * 
 * The following sample code shows how you can:
 *  - create a packet and pass it around,
 *  - add and remove arbitrary protocol headers to a packet,
 *  - add and remove arbitrary tags to a packet, and,
 *  - create custom tags and protocol headers.
 *
 * \include samples/main-packet.cc
 * 
 * Rationale:
 * This design was inpired by the need to make our packets look like and 
 * behave as close as possible to real-world packets. The best way to achieve
 * this is to make our Packets be just real-world packets and this is exactly
 * what this class does. It behaves very similarly to the linux skbuf and the
 * bsd mbuf data structures.
 */
class Packet {
public:
	typedef Callback<void,uint8_t *,uint32_t> PacketReadWriteCallback;

	/**
	 * Create an empty Packet.
	 */
	static Packet *create (void);

	/**
	 * Create a copy of this Packet together with its tags.
	 */
	Packet *copy (void) const;
	/**
	 * \param start offset from start of packet
	 * \param length of new fragment
	 * \return the fragment
	 *
	 * Create a copy of a fragment of this Packet. The tags are
	 * entirely copied to the new fragment. 
	 */
	Packet *copy (uint32_t start, uint32_t length) const;

	/**
	 * \return the size of the Packet.
	 */
	uint32_t get_size (void) const;

	/**
	 * \return a pointer to the internal byte buffer stored
	 *         in this Packet.
	 *
	 * We strongly advise you against the use of this 
	 * method. It is currently used in only a few very rare
	 * places and the number of uses should not grow.
	 */
	uint8_t *peek_data (void) const;

	/**
	 * \param tag tag to add to packet.
	 *
	 * Make a copy of the input tag and store it in the Packet.
	 */
	template <typename T>
	void add_tag (T const *tag);
	/**
	 * \param tag tag to remove from packet.
	 *
	 * Copy and remove the tag stored in this Packet identified
	 * by the input tag type.
	 */
	template <typename T>
	bool remove_tag (T *tag);
	/**
	 * \param tag tag to copy
	 *
	 * Copy the tag stored in this Packet identified by the
	 * input tag type.
	 */
	template <typename T>
	bool peek_tag (T *tag) const;
	/**
	 * \param tag tag to update
	 *
	 * Copy this tag into the Packet.
	 */
	template <typename T>
	bool update_tag (T const*tag);

	/**
	 * \param chunk chunk to add to Packet
	 *
	 * The Chunk::add method will be invoked. It is responsible for:
	 *   - reserving room for the data to serialize
	 *   - serialize its data in the reserved room
	 */
	void add (Chunk *chunk);
	/**
	 * \param packet input packet to append to this packet
	 *
	 * Append the content of the input packet to this packet.
	 */
	void add_at_end (ConstPacketPtr packet);
	/**
	 * \param packet input packet to append to this packet
	 * \param offset offset from start of input packet to start copying from
	 * \param size number of bytes to append to this packet
	 *
	 * Append a slice of the content of the input packet to this packet.
	 * This method is more efficient than creating a fragment of the input
	 * packet first and then invoke add_at_end on the fragment.
	 */
	void add_at_end (ConstPacketPtr packet, uint32_t offset, uint32_t size);
	/**
	 * \param chunk to read from packet
	 * 
	 * The Chunk::peek_from method will be invoked. Its job is to
	 * deserialize the Chunk from the internal byte buffer.
	 */
	void peek (Chunk *chunk) const;
	/**
	 * \param chunk chunk to remove from packet
	 *
	 * The Chunk::remove_from method will be invoked. Its job is to
	 * remove the serialized representation of the Chunk from the
	 * internal byte buffer.
	 */
	void remove (Chunk *chunk);
	/**
	 * \param size number of bytes to remove
	 * 
	 * Remove size bytes from the end of the Packet.
	 */
	void remove_at_end (uint32_t size);
	/**
	 * \param size number of bytes to remove
	 *
	 * Remove size bytes from the start of the Packet.
	 */
	void remove_at_start (uint32_t size);

	void write (PacketReadWriteCallback callback) const;
	void read (PacketReadWriteCallback callback, uint32_t to_read);

 private:
	friend class RefPtr<Packet>;
	friend class RefPtr<Packet const>;
	friend class GPacket;
	void destroy (void) const; // called by RefPtr
	void reset (void);
	Packet ();
	~Packet ();

	mutable uint32_t m_count;
	Buffer *m_buffer;
	GTags m_tags;
};

std::ostream& operator<< (std::ostream& os, Packet const& packet);

}; // namespace yans

namespace yans {

template <typename T>
void Packet::add_tag (T const*tag)
{
	m_tags.add (tag);
}
template <typename T>
bool Packet::remove_tag (T *tag)
{
	return m_tags.remove (tag);
}
template <typename T>
bool Packet::peek_tag (T *tag) const
{
	return m_tags.peek (tag);
}
template <typename T>
bool Packet::update_tag (T const*tag)
{
	return m_tags.update (tag);
}


}; // namespace yans

#endif /* PACKET_H */
