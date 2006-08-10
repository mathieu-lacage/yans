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

namespace yans {

GBuffer::GBufferDataList  GBuffer::m_free_list;
uint32_t GBuffer::m_prefered_size = 0;
uint32_t GBuffer::m_prefered_start = 0;

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
	return ok;
}



static GBufferTest g_buffer_test;

}; // namespace yans

#endif /* RUN_SELF_TESTS */


