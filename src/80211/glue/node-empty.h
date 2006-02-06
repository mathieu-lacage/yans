/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2005 INRIA
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

#ifndef NODE_EMPTY_H
#define NODE_EMPTY_H

#include "node.h"

class Agent;
class Classifier;


class NodeEmpty : public TclObject {
public:
	NodeEmpty ();
	virtual ~NodeEmpty ();

	int getAddress (void);

	int command(int argc, const char*const* argv);
private:
	void attachAgent (Agent *agent);
	int allocUid (void);

	static int m_uid;
	int m_address;
	Classifier *m_demux;

	double m_x;
	double m_y;
	double m_z;

	double m_speedX;
	double m_speedY;
	double m_speedZ;
};

#endif /* NODE_EMPTY_H */
