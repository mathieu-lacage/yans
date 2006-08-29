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

#ifndef READER_H
#define READER_H

#include "yans/stdint.h"

namespace yans {

class ReaderBuffer {
 public:
        ReaderBuffer (int fd);
        ~ReaderBuffer ();
        void read (uint8_t *buffer, uint32_t size);
        void clear_errors (void);
        bool is_error (void);
        uint32_t get_offset   (void) const;
        void     seek         (uint32_t offset);
        void     skip         (uint32_t offset);
        void     skip64       (uint64_t offset);
 private:
        int m_fd;
        bool m_is_error;
};

class Reader {
 public:
        Reader (ReaderBuffer *buffer);
        ~Reader ();

        void clear_errors (void);
        bool is_error (void) const;
        void set_msb (void);
        void set_lsb (void);
        uint8_t read_u8 (void);
        uint16_t read_u16 (void);
        uint32_t read_u32 (void);
        uint64_t read_u64 (void);
        int8_t read_s8 (void);
        int16_t read_s16 (void);
        int32_t read_s32 (void);
        int64_t read_s64 (void);
        uint32_t read_u (uint8_t length);
        uint64_t read_uleb128 (void);
        int64_t  read_sleb128 (void);
        uint8_t  read_u8bcd   (void);
        uint16_t read_u16bcd  (void);

        uint32_t get_offset   (void) const;
        /* absolute offset. */
        void     seek         (uint32_t offset);
        /* relative offset. */
        void     skip         (uint32_t offset);
        void     skip64       (uint64_t offset);
 private:
        bool m_lsb;
        bool m_is_error;
        ReaderBuffer *m_buffer;
};

}; // namespace yans

#endif /* READER_H */
