/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#include "yans/packet.h"
#include "yans/chunk.h"
#include "yans/buffer.h"
#include "yans/tag.h"
#include <iostream>

using namespace yans;

/* A sample Chunk implementation
 */
class MyChunk : public Chunk {
public:
	MyChunk ();
	virtual ~MyChunk ();

	void set_data (uint16_t data);
	uint16_t get_data (void) const;
private:
	virtual void add_to (Buffer *buffer) const;
	virtual void peek_from (Buffer const*buffer);
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;
	virtual void add_to (GBuffer *buffer) const;
	virtual void peek_from (GBuffer const *buffer);
	virtual void remove_from (GBuffer *buffer);

	uint16_t m_data;
};

MyChunk::MyChunk ()
{}
MyChunk::~MyChunk ()
{}
void 
MyChunk::add_to (Buffer *buffer) const
{
	// reserve 2 bytes at head of buffer
	buffer->add_at_start (2);
	Buffer::Iterator i = buffer->begin ();
	// serialize in head of buffer
	i.write_hton_u16 (m_data);
}
void 
MyChunk::peek_from (Buffer const*buffer)
{
	Buffer::Iterator i = buffer->begin ();
	// deserialize from head of buffer
	m_data = i.read_ntoh_u16 ();
}
void 
MyChunk::remove_from (Buffer *buffer)
{
	// remove deserialized data
	buffer->remove_at_start (2);
}
void 
MyChunk::print (std::ostream *os) const
{
	*os << "MyChunk data=" << m_data << std::endl;
}
void 
MyChunk::add_to (GBuffer *buffer) const
{
	// reserve 2 bytes at head of buffer
	buffer->add_at_start (2);
	GBuffer::Iterator i = buffer->begin ();
	// serialize in head of buffer
	i.write_hton_u16 (m_data);
}
void 
MyChunk::peek_from (GBuffer const *buffer)
{
	GBuffer::Iterator i = buffer->begin ();
	// deserialize from head of buffer
	m_data = i.read_ntoh_u16 ();
}
void 
MyChunk::remove_from (GBuffer *buffer)
{
	// remove deserialized data
	buffer->remove_at_start (2);
}

void 
MyChunk::set_data (uint16_t data)
{
	m_data = data;
}
uint16_t 
MyChunk::get_data (void) const
{
	return m_data;
}

/* A sample Tag implementation
 */
class MyTag : public Tag {
public:
	MyTag ();
	virtual ~MyTag ();
	void set_stream_id (uint16_t stream_id);
	uint16_t get_stream_id (void);
private:
	virtual uint32_t real_get_id (void) const;
	virtual uint32_t real_get_size (void) const;
	uint16_t m_stream_id;
};

MyTag::MyTag ()
{}
MyTag::~MyTag ()
{}
void 
MyTag::set_stream_id (uint16_t stream_id)
{
	m_stream_id = stream_id;
}
uint16_t 
MyTag::get_stream_id (void)
{
	return m_stream_id;
}
uint32_t 
MyTag::real_get_id (void) const
{
	static uint32_t id = TagManager::register_tag ("MyTag");
	return id;
}
uint32_t 
MyTag::real_get_size (void) const
{
	return sizeof (*this);
}


static void
receive (PacketPtr p)
{
	MyChunk my;
	p->peek (&my);
	p->remove (&my);
	std::cout << "received data=" << my.get_data () << std::endl;
	MyTag my_tag;
	p->remove_tag (&my_tag);
}


int main (int argc, char *argv[])
{
	PacketPtr p = Packet::create ();
	MyChunk my;
	my.set_data (2);
	std::cout << "send data=2" << std::endl;
	p->add (&my);
	MyTag my_tag;
	my_tag.set_stream_id (5);
	p->add_tag (&my_tag);
	receive (p);
	return 0;
}
