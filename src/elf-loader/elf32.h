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

/* segment type */
enum {
	PT_NULL     = 0,
	PT_LOAD     = 1,
	PT_DYNAMIC  = 2,
	PT_INTERP   = 3,
	PT_NOTE     = 4,
	PT_SHLIB    = 5,
	PT_PHDR     = 6,
	PT_TLS      = 7,
	PT_NUM      = 8,
	PT_LOOS    = 0x60000000,
	PT_GNU_EH_FRAME = 0x6474e550,
	PT_GNU_STACK    = 0x6474e551,
	PT_GNU_REL0     = 0x6474e552,
	PT_HIOS    = 0x6fffffff,
	PT_LOPROC  = 0x70000000,
	PT_HIPROC  = 0x7fffffff
};

/* segment permissions */
enum {
	PF_X        = 0x1,
	PF_W        = 0x2,
	PF_R        = 0x4,
	PF_MASKOS   = 0x0ff00000,
	PF_MASKPROC = 0xf0000000
};

enum {
        STT_NOTYPE  = 0,
        STT_OBJECT  = 1,
        STT_FUNC    = 2,
        STT_SECTION = 3,
        STT_FILE    = 4,
        STT_COMMON  = 5,
        STT_TLS     = 6,
        STT_LOOS    = 10,
        STT_HIOS    = 12,
        STT_LOPROC  = 13,
        STT_HIPROC  = 15
};

enum {
	SHN_UNDEF      = 0,
	SHN_LORESERVE  = 0xff00,
	SHN_LOPROC     = 0xff00,
	SHN_HIPROC     = 0xff1f,
	SHN_LOOS       = 0xff20,
	SHN_HIOS       = 0xff3f,
	SHN_ABS        = 0xfff1,
	SHN_COMMON     = 0xfff2,
	SHN_XINDEX     = 0xffff,
	SHN_HIRESERVE  = 0xffff
};

struct elf32_symbol {
        uint32_t st_name;
        uint32_t st_value;
        uint32_t st_size;
        uint8_t st_info;
        uint8_t st_other;
        uint16_t st_shndx;
};                                                                                                                                                             
struct elf32_section_header {
        uint32_t sh_name;
        uint32_t sh_type;
        uint32_t sh_flags;
        uint32_t sh_addr;
        uint32_t sh_offset;
        uint32_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint32_t sh_addralign;
        uint32_t sh_entsize;
};

struct elf32_program_header {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
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
