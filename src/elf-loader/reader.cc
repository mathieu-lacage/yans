/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  Copyright (C) 2004,2005 Mathieu Lacage
  Copyright (C) 2006 INRIA

  Author: Mathieu Lacage <mathieu@gnu.org>
          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
  
  Originally, code written for bozo-profiler in C. Ported to c++ only later.
*/
#include "reader.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>


namespace yans {

ReaderBuffer::ReaderBuffer (int fd)
        : m_fd (fd), 
        m_is_error (false) {}
ReaderBuffer::~ReaderBuffer ()
{}
void 
ReaderBuffer::read (uint8_t *buffer, uint32_t size)
{
        if (m_is_error) {
                return;
        }
        errno = 0;
        ssize_t bytes_read = ::read (m_fd, buffer, size);
        if (bytes_read == -1) {
                m_is_error = true;
                printf ("error reading file: \"%s\"", strerror (errno));
        }
}
void 
ReaderBuffer::clear_errors (void)
{
        m_is_error = false;
}
bool 
ReaderBuffer::is_error (void)
{
        return m_is_error;
}

uint32_t 
ReaderBuffer::get_offset (void) const
{
        off_t offset = ::lseek (m_fd, 0, SEEK_SET);
        return offset;
}
void
ReaderBuffer::seek (uint32_t offset)
{
        ::lseek (m_fd, offset, SEEK_SET);
}
void     
ReaderBuffer::skip (uint32_t offset)
{
        ::lseek (m_fd, offset, SEEK_CUR);
}
void     
ReaderBuffer::skip64 (uint64_t offset)
{
        ::lseek (m_fd, offset, SEEK_CUR);
}



Reader::Reader (ReaderBuffer *buffer)
        : m_lsb  (true),
        m_is_error (false),
        m_buffer (buffer)
{}
Reader::~Reader ()
{}

void 
Reader::clear_errors (void)
{
        m_is_error = false;
        m_buffer->clear_errors ();
}
bool 
Reader::is_error (void) const
{
        return m_is_error || m_buffer->is_error ();
}
void 
Reader::set_msb (void)
{
        m_lsb = false;
}
void 
Reader::set_lsb (void)
{
        m_lsb = true;
}

uint8_t 
Reader::read_u8  (void)
{
        uint8_t retval = 0xff;
        m_buffer->read (&retval, 1);
        return retval;
}

uint16_t 
Reader::read_u16 (void)
{
        uint8_t buffer[2] = {0xff, 0xff};
        uint16_t retval;
        m_buffer->read (buffer, 2);
        if (m_lsb) {
                retval = buffer[0] + (buffer[1]<<8);
        } else {
                retval = (buffer[0]<<8) + buffer[1];
        }
        return retval;
}

uint32_t
Reader::read_u32 (void)
{
        uint8_t buffer[4] = {0xff, 0xff, 0xff, 0xff};
        uint32_t retval;
        m_buffer->read (buffer, 4);
        if (m_lsb) {
                retval = buffer[0] + (buffer[1]<<8) + (buffer[2]<<16) + (buffer[3]<<24);
        } else {
                retval = (buffer[0]<<24) + (buffer[1]<<16) + (buffer[2]<<8) + buffer[3];
        }
        return retval;
}

uint64_t
Reader::read_u64 (void)
{
        uint8_t buffer[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        uint64_t retval = 0;
        m_buffer->read (buffer, 8);
        if (m_lsb) {
                retval |= buffer[7];
                retval <<= 8;
                retval |= buffer[6];
                retval <<= 8;
                retval |= buffer[5];
                retval <<= 8;
                retval |= buffer[4];
                retval <<= 8;
                retval |= buffer[3];
                retval <<= 8;
                retval |= buffer[2];
                retval <<= 8;
                retval |= buffer[1];
                retval <<= 8;
                retval |= buffer[0];
                retval <<= 8;
        } else {
                retval |= buffer[0];
                retval <<= 8;
                retval |= buffer[1];
                retval <<= 8;
                retval |= buffer[2];
                retval <<= 8;
                retval |= buffer[3];
                retval <<= 8;
                retval |= buffer[4];
                retval <<= 8;
                retval |= buffer[5];
                retval <<= 8;
                retval |= buffer[6];
                retval <<= 8;
                retval |= buffer[7];
                retval <<= 8;
        }
        return retval;
}

uint64_t
Reader::read_uleb128 (void)
{
        uint64_t result = 0;
        uint8_t byte;
        uint8_t shift = 0;
        do {
                m_buffer->read (&byte, 1);
                result |= (byte & (~0x80))<<shift;
                shift += 7;
        } while (byte & 0x80 && 
                 /* a LEB128 unsigned number is at most 9 bytes long. */
                 shift < (7*9)); 
        if (byte & 0x80) {
                /* This means the LEB128 number was not valid.
                 * ie: the last (9th) byte did not have the high-order bit zeroed.
                 */
                m_is_error = true;
        }
        return result;
}

int64_t 
Reader::read_sleb128 (void)
{
        int64_t result = 0; 
        uint8_t shift = 0;
        uint8_t byte = 0;
        while(true) { 
                m_buffer->read (&byte, 1);
                result |= (((uint64_t)(byte & 0x7f)) << shift); 
                shift += 7; /* sign bit of byte is 2nd high order bit (0x40) */ 
                if ((byte & 0x80) == 0) {
                        break; 
                }
        } 
        if ((shift < 64) && (byte & 0x40)) {
                /* sign extend */ 
                result |= - (1 << shift);
        }
        return result;
}

int8_t
Reader::read_s8 (void)
{
        return static_cast<int8_t> (read_u8 ());
}

int16_t
Reader::read_s16 (void)
{
        return static_cast<int16_t> (read_u16 ());
}

int32_t
Reader::read_s32 (void)
{
        return static_cast<int32_t> (read_u32 ());
}

int64_t
Reader::read_s64 (void)
{
        return static_cast<int64_t> (read_u64 ());
}

uint8_t
Reader::read_u8bcd (void)
{
        uint8_t retval;
        uint8_t byte = 0;
        uint8_t bcd0, bcd1;
        m_buffer->read (&byte, 1);
        bcd0 = byte & 0xf;
        bcd1 = byte >> 4;

        // XXX Probably wrong. I have no idea what I am doing here.
        if (m_lsb) {
                retval = bcd0 + bcd1*10;
        } else {
                retval = bcd0 + bcd1*10;
        }

        return retval;
}

uint16_t
Reader::read_u16bcd (void)
{
        uint16_t retval;
        uint8_t buffer[2] = {0, 0};
        uint8_t bcd0, bcd1, bcd2, bcd3;
        m_buffer->read (buffer, 2);
        bcd0 = buffer[0] & 0xf;
        bcd1 = buffer[0] >> 4;
        bcd2 = buffer[1] & 0xf;
        bcd3 = buffer[1] >> 4;

        if (m_lsb) {
                // XXX untested. Probably wrong.
                retval = bcd0 + bcd1*10 + bcd2*100 + bcd3*1000;
        } else {
                retval = bcd1*1000 + bcd0*100 + bcd3*10 + bcd2;
        }

        return retval;
}

uint32_t 
Reader::get_offset (void) const
{
        return m_buffer->get_offset ();
}
void
Reader::seek (uint32_t offset)
{
        m_buffer->seek (offset);
}
void     
Reader::skip (uint32_t offset)
{
        m_buffer->skip (offset);
}
void     
Reader::skip64 (uint64_t offset)
{
        m_buffer->skip64 (offset);
}


}; // namespace yans
