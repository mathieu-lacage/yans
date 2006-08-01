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
#ifndef ELF_PROCESS_H
#define ELF_PROCESS_H

#include <string>
#include <vector>

struct Libc;

namespace yans {

class ElfProcess {
public:
	ElfProcess (char const *root_path, char const *binary);
	~ElfProcess ();

	void add_arg (char const *arg);
	void start (int argc, char *argv[]);
	void start (void);
private:
	typedef std::vector<char *> Args;
	typedef std::vector<char *>::iterator ArgsI;
	struct Libc *m_libc;
	std::string m_binary;
	Args m_args;
	void *m_module;
};

}; // namespace yans

#endif /* ELF_PROCESS_H */
