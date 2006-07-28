/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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

#include "elf-module.h"
#include "reader.h"

// ::open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERROR_REPORT

#ifdef ERROR_REPORT
  #include <iostream>
  #define REPORT(data) \
    std::cout << "DEBUG "<<__FILE__<<":"<<__LINE__<<" "<<data<<std::endl;
#else
  #define REPORT(data)
#endif

#define ASCII_E 0x45
#define ASCII_L 0x4C
#define ASCII_F 0x46



namespace yans {

ElfModule::ElfModule (char const *filename, char const *root_dir)
	: m_filename (filename),
	  m_root_dir (root_dir)
{}

ElfModule::~ElfModule ()
{}

void
ElfModule::run (void)
{
	int fd = ::open (m_filename.c_str (), O_RDONLY);
	ReaderBuffer buffer = ReaderBuffer (fd);
	Reader reader = Reader (&buffer);
	reader.seek (0);
	uint8_t b = reader.read_u8 ();
	if (b != 0x7f) {
                REPORT ("invalid ELF magic number");
                goto error;		
	}
        b = reader.read_u8 ();
        if (b != ASCII_E) {
                REPORT ("invalid ELF E");
                goto error;
        }
        b = reader.read_u8 ();
        if (b != ASCII_L) {
                REPORT ("invalid ELF L");
                goto error;
        }
        b = reader.read_u8 ();
        if (b != ASCII_F) {
                REPORT ("invalid ELF F");
                goto error;
        }

 error:
	return;
}

}; // namespace yans
