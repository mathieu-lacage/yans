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
#include "elf-process.h"
#include "libc.h"
#include <dlfcn.h>
#include <iostream>

namespace yans {

ElfProcess::ElfProcess (char const *root_path, char const *binary)
{
	m_libc = libc_new (root_path);
	m_binary = binary;
}
ElfProcess::~ElfProcess ()
{
	libc_delete (m_libc);
	for (ArgsI i = m_args.begin (); i != m_args.end (); i++) {
		free (*i);
	}
	m_args.erase (m_args.begin (), m_args.end ());
}

void 
ElfProcess::add_arg (char const *arg)
{
	m_args.push_back (strdup (arg));
}

void 
ElfProcess::start (int argc, char *argv[])
{
	m_module = dlmopen (LM_ID_NEWLM, m_binary.c_str (), RTLD_NOW);	
	if (m_module == 0) {
		std::cerr << "error=\""<< dlerror ()<<"\""<<std::endl;
		return;
	}
	void *symbol = dlsym (m_module, "set_libc");
	if (symbol == 0) {
		std::cerr << "error lookup set_libc."<<std::endl;
		return;
	}
	void (*set_libc) (struct Libc *,struct libc_FILE*,struct libc_FILE*,struct libc_FILE*) = 
		(void (*) (struct Libc *,struct libc_FILE*,struct libc_FILE*,struct libc_FILE*))symbol;
	(*set_libc) (m_libc, m_libc->stdin, m_libc->stdout, m_libc->stderr);
}

void
ElfProcess::start (void)
{
	char **argv;
	argv = (char **)malloc (sizeof (char **) * m_args.size ());
	for (unsigned int i = 0; i < m_args.size (); i++) {
		argv[i] = m_args[i];
	}
	start (m_args.size (), argv);
}

}; // namespace yans
