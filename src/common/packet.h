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
#include "ref-ptr.h"

namespace yans {

class Packet;
class Chunk;
class Buffer;
class Tag;
class Tags;

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
 * Network Packets are represented by an array of bytes. The interpretation
 * of the content of that array of bytes is entirely up to the user since
 * the Packet data structure has no knowledge of what data was stored in it.
 * 
 * This design was inpired by the need to make our packets look like and 
 * behave as close as possible to real-world packets. The best way to achieve
 * this is to make our Packets be just real-world packets and this is exactly
 * what this class does. It behaves very similarly to the linux skbuf and the
 * bsd mbuf data structures.
 *
 * It is also possible to attach a single instance of any number of Tag types
 * to each Packet: this makes it easy to attach per-packet data to exchange 
 * information across multiple network layers. 
 */
class Packet {
public:
	typedef Callback<void,uint8_t *,uint32_t> PacketReadWriteCallback;

	/**
	 * Create an empty Packet.
	 */
	static PacketPtr create (void);

	/**
	 * Create a copy of this Packet together with its tags.
	 */
	PacketPtr copy (void) const;
	/**
	 * \param start offset from start of packet
	 * \param length of new fragment
	 * \return the fragment
	 *
	 * Create a copy of a fragment of this Packet. The tags are
	 * entirely copied to the new fragment. 
	 */
	PacketPtr copy (uint32_t start, uint32_t length) const;

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
	void add_tag (Tag const*tag);
	/**
	 * \param tag tag to remove from packet.
	 *
	 * Copy and remove the tag stored in this Packet identified
	 * by the input tag type.
	 */
	void remove_tag (Tag *tag);
	/**
	 * \param tag tag to copy
	 *
	 * Copy the tag stored in this Packet identified by the
	 * input tag type.
	 */
	void peek_tag (Tag *tag);
	/**
	 * \param tag tag to update
	 *
	 * Copy this tag into the Packet.
	 */
	void update_tag (Tag *tag);

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
	 * \param chunk chunk to remove from packet
	 *
	 * The Chunk::add method will be invoked. It is responsible for:
	 *   - deserializing the corresponding protocol header into
	 *     the input chunk
	 *   - remove the data deserialized from the Packet.
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
	static void recycle (Packet *packet);
	void reset (void);
	void ref (void) const;
	void unref (void) const;
	Packet ();
	~Packet ();

	mutable uint32_t m_count;
	Buffer *m_buffer;
	Tags *m_tags;
};

std::ostream& operator<< (std::ostream& os, Packet const& packet);

}; // namespace yans

#endif /* PACKET_H */
