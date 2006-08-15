/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "gbuffer.h"
#include <cassert>

namespace yans {

GBuffer::GBufferDataList  GBuffer::m_free_list;
uint32_t GBuffer::m_prefered_size = 1;
uint32_t GBuffer::m_prefered_start = 0;

struct GBuffer::GBufferData *
GBuffer::allocate (uint32_t req_size, uint32_t req_start)
{
	assert (req_size >= 1);
	uint32_t size = req_size - 1 + sizeof (struct GBuffer::GBufferData);
	uint8_t *b = new uint8_t [size];
	struct GBufferData *data = reinterpret_cast<struct GBuffer::GBufferData*>(b);
	data->m_size = req_size;
	data->m_dirty_start = req_start;
	data->m_dirty_size = 0;
	data->m_count = 1;
	return data;
}

void
GBuffer::deallocate (struct GBuffer::GBufferData *data)
{
	uint8_t *buf = reinterpret_cast<uint8_t *> (data);
	delete [] buf;
}
#ifdef USE_FREE_LIST
void
GBuffer::recycle (struct GBuffer::GBufferData *data)
{
	assert (data->m_count == 0);
	/* get rid of it if it is too small for later reuse. */
	if (data->m_size < GBuffer::m_prefered_size) {
		GBuffer::deallocate (data);
		return; 
	}
	/* update buffer statistics */
	uint32_t cur_prefered_end = GBuffer::m_prefered_size - GBuffer::m_prefered_start;
	if (m_total_added_start > GBuffer::m_prefered_start) {
		GBuffer::m_prefered_start = m_total_added_start;
	}
	uint32_t prefered_end;
	if (m_total_added_end > cur_prefered_end) {
		prefered_end = m_total_added_end;
	} else {
		prefered_end = cur_prefered_end;
	}
	GBuffer::m_prefered_size = GBuffer::m_prefered_start + prefered_end;
	assert (GBuffer::m_prefered_size >= GBuffer::m_prefered_start);
	/* feed into free list */
	if (GBuffer::m_free_list.size () > 1000) {
		GBuffer::deallocate (data);
	} else {
		GBuffer::m_free_list.push_back (data);
	}
}

GBuffer::GBufferData *
GBuffer::create (void)
{
	/* try to find a buffer correctly sized. */
	while (!GBuffer::m_free_list.empty ()) {
		struct GBuffer::GBufferData *data = GBuffer::m_free_list.back ();
		GBuffer::m_free_list.pop_back ();
		if (data->m_size > GBuffer::m_prefered_size) {
			assert (GBuffer::m_prefered_size >= GBuffer::m_prefered_start);
			data->m_dirty_start = GBuffer::m_prefered_start;
			data->m_dirty_size = 0;
			data->m_count = 1;
			return data;
		}
		GBuffer::deallocate (data);
	}
	struct GBuffer::GBufferData *data = GBuffer::allocate (GBuffer::m_prefered_size, 
							       GBuffer::m_prefered_start);
	assert (data->m_count == 1);
	return data;
}
#else
void
GBuffer::recycle (struct GBuffer::GBufferData *data)
{
	GBuffer::deallocate (data);
}

GBuffer::GBufferData *
GBuffer::create (void)
{
	return GBuffer::allocate (GBuffer::m_prefered_size, GBuffer::m_prefered_start);
}
#endif

}; // namespace yans




#ifdef RUN_SELF_TESTS

#include "test.h"
#include <iomanip>

namespace yans {

class GBufferTest: public Test {
private:
  bool ensure_written_bytes (GBuffer::Iterator i, uint32_t n, uint8_t array[]);
public:
  virtual bool run_tests (void);
  GBufferTest ();
};


GBufferTest::GBufferTest ()
	: Test ("GBuffer") {}

bool
GBufferTest::ensure_written_bytes (GBuffer::Iterator i, uint32_t n, uint8_t array[])
{
	bool success = true;
	uint8_t *expected = array;
	uint8_t *got;
	got = i.peek_data ();
	for (uint32_t j = 0; j < n; j++) {
		if (got[j] != expected[j]) {
			success = false;
		}
	}
	if (!success) {
		failure () << "Buffer -- ";
		failure () << "expected: ";
		failure () << n << " ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t j = 0; j < n; j++) {
			failure () << (uint16_t)expected[j] << " ";
		}
		failure ().setf (std::ios::dec, std::ios::basefield);
		failure () << "got: ";
		failure ().setf (std::ios::hex, std::ios::basefield);
		for (uint32_t j = 0; j < n; j++) {
			failure () << (uint16_t)got[j] << " ";
		}
		failure () << std::endl;
	}
	return success;
}

/* Note: works only when variadic macros are
 * available which is the case for gcc.
 * XXX
 */
#define ENSURE_WRITTEN_BYTES(iterator, n, ...) \
{ \
	uint8_t bytes[] = {__VA_ARGS__}; \
	if (!ensure_written_bytes (iterator, n , bytes)) { \
		ok = false; \
	} \
}

bool
GBufferTest::run_tests (void)
{
	bool ok = true;
	GBuffer buffer;
	GBuffer::Iterator i;
	buffer.add_at_start (6);
	i = buffer.begin ();
	i.write_u8 (0x66);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 1, 0x66);
	i = buffer.begin ();
	i.write_u8 (0x67);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 1, 0x67);
	i.write_hton_u16 (0x6568);
	i = buffer.begin ();
	ENSURE_WRITTEN_BYTES (buffer.begin (), 3, 0x67, 0x65, 0x68);
	i.write_hton_u16 (0x6369);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 3, 0x63, 0x69, 0x68);
	i.write_hton_u32 (0xdeadbeaf);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 6, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf);
	buffer.add_at_start (2);
	i = buffer.begin ();
	i.write_u16 (0);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 8, 0, 0, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf);
	buffer.add_at_end (2);
	i = buffer.begin ();
	i.next (8);
	i.write_u16 (0);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 10, 0, 0, 0x63, 0x69, 0xde, 0xad, 0xbe, 0xaf, 0, 0);
	buffer.remove_at_start (3);
	i = buffer.begin ();
	ENSURE_WRITTEN_BYTES (buffer.begin (), 7, 0x69, 0xde, 0xad, 0xbe, 0xaf, 0, 0);
	buffer.remove_at_end (4);
	i = buffer.begin ();
	ENSURE_WRITTEN_BYTES (buffer.begin (), 3, 0x69, 0xde, 0xad);
	buffer.add_at_start (1);
	i = buffer.begin ();
	i.write_u8 (0xff);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 4, 0xff, 0x69, 0xde, 0xad);
	buffer.add_at_end (1);
	i = buffer.begin ();
	i.next (4);
	i.write_u8 (0xff);
	i.prev (2);
	uint16_t saved = i.read_u16 ();
	i.prev (2);
	i.write_hton_u16 (0xff00);
	i.prev (2);
	if (i.read_ntoh_u16 () != 0xff00) {
		ok = false;
	}
	i.prev (2);
	i.write_u16 (saved);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 5, 0xff, 0x69, 0xde, 0xad, 0xff);
	GBuffer o = buffer;
	ENSURE_WRITTEN_BYTES (o.begin (), 5, 0xff, 0x69, 0xde, 0xad, 0xff);
	o.add_at_start (1);
	i = o.begin ();
	i.write_u8 (0xfe);
	ENSURE_WRITTEN_BYTES (o.begin (), 6, 0xfe, 0xff, 0x69, 0xde, 0xad, 0xff);
	buffer.add_at_start (2);
	i = buffer.begin ();
	i.write_u8 (0xfd);
	i.write_u8 (0xfd);
	ENSURE_WRITTEN_BYTES (o.begin (), 6, 0xfe, 0xff, 0x69, 0xde, 0xad, 0xff);
	ENSURE_WRITTEN_BYTES (buffer.begin (), 7, 0xfd, 0xfd, 0xff, 0x69, 0xde, 0xad, 0xff);

	o = o;
	return ok;
}



static GBufferTest g_buffer_test;

}; // namespace yans

#endif /* RUN_SELF_TESTS */


