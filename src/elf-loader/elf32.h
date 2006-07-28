/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005,2006 Mathieu Lacage
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
 * Author: Mathieu Lacage <mathieu@gnu.org>
 * ripped out of bozo-profiler
 */
#ifndef ELF32_H
#define ELF32_H

/* version */
enum {
	EV_NONE     = 0,
	EV_CURRENT  = 1
};

/* file type */
enum {
        ET_NONE   = 0,
	ET_REL    = 1,
	ET_EXEC   = 2,
	ET_DYN    = 3,
	ET_CORE   = 4,
	ET_LOOS   = 0xfe00,
	ET_HIOS   = 0xfeff,
	ET_LOPROC = 0xff00,
	ET_HIPROC = 0xffff
};

/* machine type */
enum {
	EM_M32          = 1,
	EM_SPARC        = 2,
	EM_386          = 3,
	EM_68K          = 4,
	EM_88K          = 5,
	EM_860          = 7,
	EM_MIPS         = 8,
	EM_MIPS_RS4_BE  = 10
};


struct elf32_header {
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_entry;
        uint32_t e_phoff;
        uint32_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
};


#endif /* ELF32_H */
