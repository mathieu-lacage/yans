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

#include "channel-80211.h"

static class Channel80211Class: public TclClass {
public:
	Channel80211Class() : TclClass("Channel/Channel80211") {}
        TclObject* create(int, const char*const*) {
                return (new Channel80211 ());
        }
} class_Channel80211;


Channel80211::Channel80211 ()
{}

Channel80211::~Channel80211 ()
{}

/**
 * I feel bad about writing a subclass of Channel just so
 * that we can safely ignore these two tcl function calls.
 * Something could be said about the quality of the ns2 codebase...
 */
int 
Channel80211::command(int argc, const char*const* argv)
{
	if (argc == 3) {
		if (strcmp(argv[1], "add-node") == 0) {
			return TCL_OK;
		} else if (strcmp(argv[1], "remove-node") == 0) {
			return TCL_OK;
		}
	}
	return Channel::command(argc, argv);
}
