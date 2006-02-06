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

#include <string.h>
#include <stdio.h>

#include "node-constructor.h"
#include "node-empty.h"

static class NodeConstructorClass: public TclClass {
public:
        NodeConstructorClass() : TclClass("NodeConstructor") {}
        TclObject* create(int, const char*const*) {
                return (new NodeConstructor ());
        }
} class_NodeConstructor;



NodeConstructor::NodeConstructor ()
{}

NodeConstructor::~NodeConstructor ()
{}

int 
NodeConstructor::command(int argc, const char*const* argv)
{
	Tcl& tcl=Tcl::instance();
	if (argc == 2) {
		if (strcmp (argv[1], "create-node") == 0) {
			printf ("create node\n");
			NodeEmpty *node = static_cast <NodeEmpty *> (TclObject::New ("NodeEmpty"));
			tcl.resultf ("%s", node->name ());
			return TCL_OK;
		}
	}
	return TclObject::command (argc, argv);
}
