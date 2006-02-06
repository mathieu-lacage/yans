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

#include "simple-broadcast-channel.h"
#include "node-empty.h"
#include "simple-net-interface.h"

#define TCL_CONSTRUCTOR(cppClassName, tclClassName)       \
static class cppClassName##Class: public TclClass {       \
public:                                                   \
        cppClassName##Class() : TclClass(tclClassName) {} \
        TclObject* create(int, const char*const*) {       \
                return (new cppClassName ());             \
        }                                                 \
} class_##cppClassName;


TCL_CONSTRUCTOR (SimpleBroadcastChannel, "SimpleBroadcastChannel");
TCL_CONSTRUCTOR (SimpleNetInterfaceConstructor, "SimpleNetInterfaceConstructor");
TCL_CONSTRUCTOR (NodeEmpty, "NodeEmpty");
