/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
#include "yans/packet.h"
#include "yans/chunk.h"
#include "yans/buffer.h"
#include <iostream>

using namespace yans;

class MyChunk : public Chunk {
public:
	MyChunk ();
	virtual ~MyChunk ();
	virtual void add_to (Buffer *buffer) const;
	virtual void remove_from (Buffer *buffer);
	virtual void print (std::ostream *os) const;

	void set_data (uint16_t data);
	uint16_t get_data (void) const;
private:
	uint16_t m_data;
};

MyChunk::MyChunk ()
{}
MyChunk::~MyChunk ()
{}
void 
MyChunk::add_to (Buffer *buffer) const
{
	buffer->add_at_start (2);
	Buffer::Iterator i = buffer->begin ();
	i.write_hton_u16 (m_data);
}
void 
MyChunk::remove_from (Buffer *buffer)
{
	Buffer::Iterator i = buffer->begin ();
	m_data = i.read_ntoh_u16 ();
	buffer->remove_at_start (2);
}
void 
MyChunk::print (std::ostream *os) const
{
	*os << "MyChunk data=" << m_data << std::endl;
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

static void
receive (PacketPtr p)
{
	MyChunk my;
	p->remove (&my);
	std::cout << "received data=" << my.get_data () << std::endl;
}


int main (int argc, char *argv[])
{
	PacketPtr p = Packet::create ();
	MyChunk my;
	my.set_data (2);
	std::cout << "send data=2" << std::endl;
	p->add (&my);
	receive (p);
	return 0;
}
