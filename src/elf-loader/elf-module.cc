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

struct elf32_header
ElfModule::read_header (Reader *reader) const
{
	struct elf32_header header;
	reader->seek (0);
	uint8_t b = reader->read_u8 ();
	if (b != 0x7f) {
                REPORT ("invalid ELF magic number");
                goto error;		
	}
        b = reader->read_u8 ();
        if (b != ASCII_E) {
                REPORT ("invalid ELF E");
                goto error;
        }
        b = reader->read_u8 ();
        if (b != ASCII_L) {
                REPORT ("invalid ELF L");
                goto error;
        }
        b = reader->read_u8 ();
        if (b != ASCII_F) {
                REPORT ("invalid ELF F");
                goto error;
        }

	/* ei_class */
        b = reader->read_u8 ();
        if (b != 1) {
                REPORT ("invalid ei_class");
                goto error;
        }
        /* type */
        b = reader->read_u8 ();
        if (b == 1) {
                reader->set_lsb ();
        } else if (b == 2) {
                reader->set_msb ();
        } else {
                REPORT ("invalid byte order type");
                goto error;
        }
        /* ei_version */
        b = reader->read_u8 ();
        if (b != EV_CURRENT) {
                REPORT ("invalid ei_version");
                goto error;
        }
        reader->skip (9);
        header.e_type = reader->read_u16 ();
        if (header.e_type == ET_NONE) {
                REPORT ("no file type");
                goto error;
        } else if (header.e_type > ET_CORE &&
                   header.e_type < ET_LOOS) {
                REPORT ("unknown file type: "<<header.e_type);
                goto error;
        } else if (header.e_type >= ET_LOOS &&
                   header.e_type <= ET_HIOS) {
                REPORT ("os-specific file");
                goto error;
        } else if (header.e_type >= ET_LOPROC) {
                REPORT ("processor-specific file");
                goto error;
        }
        header.e_machine = reader->read_u16 ();
        if (header.e_machine != EM_386) {
                REPORT ("untested machine type "<<header.e_machine);
                goto error;
        }

        reader->skip (4);
        header.e_entry = reader->read_u32 ();
        header.e_phoff = reader->read_u32 ();
        header.e_shoff = reader->read_u32 ();
        header.e_flags = reader->read_u32 ();
        header.e_ehsize = reader->read_u16 ();
        header.e_phentsize = reader->read_u16 ();
        header.e_phnum = reader->read_u16 ();
        header.e_shentsize = reader->read_u16 ();
        header.e_shnum = reader->read_u16 ();
        header.e_shstrndx = reader->read_u16 ();

	return header;
 error:
	return header;
}

void
ElfModule::run (void)
{
	int fd = ::open (m_filename.c_str (), O_RDONLY);
	ReaderBuffer buffer = ReaderBuffer (fd);
	Reader reader = Reader (&buffer);

	struct elf32_header elf32_header = read_header (&reader);
	
}

}; // namespace yans
