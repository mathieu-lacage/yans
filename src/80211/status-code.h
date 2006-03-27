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
#ifndef STATUS_CODE_H
#define STATUS_CODE_H

#include <stdint.h>

namespace yans {

class Buffer;

class StatusCode {
public:
	StatusCode ();
	void set_success (void);
	void set_failure (void);

	bool is_success (void);

	uint32_t get_size (void) const;
	void write_to (Buffer *buffer) const;
	uint32_t read_from (Buffer *buffer);
private:
	uint16_t m_code;
};

}; // namespace yans

#endif /* REASON_CODE_H */
