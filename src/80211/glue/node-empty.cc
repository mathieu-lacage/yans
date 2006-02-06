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

#include "classifier.h"

#include "node-empty.h"
#include "broadcast-channel.h"
#include "net-interface-constructor.h"

namespace {
	class InterfaceConnector : public NsObject {
	public:
		InterfaceConnector (NodeEmpty *node)
			: m_node (node) {}
		virtual ~InterfaceConnector () {}

		virtual void recv(Packet*packet, Handler* callback = 0) {
			// XXX we don't deal with packet callbacks here.
			m_node->sendDown (packet);
		}
	private:
		NodeEmpty *m_node;
	};
}

int NodeEmpty::m_uid = 0;

NodeEmpty::NodeEmpty ()
	: m_address (allocUid ()),
	  m_demux (0)
{
	m_interfaceConnector = new InterfaceConnector (this);
	m_entry = static_cast <Classifier *> (TclObject::New ("Classifier/Addr"));
	m_demux = static_cast <Classifier *> (TclObject::New ("Classifier/Port"));
	// add notification route ?
}

NodeEmpty::~NodeEmpty ()
{}

int
NodeEmpty::allocUid (void)
{
	m_uid++;
	return m_uid;
}

int
NodeEmpty::getAddress (void)
{
	return m_address;
}

void
NodeEmpty::sendDown (Packet *packet)
{
	m_interface->sendDown (packet);
}

void 
NodeEmpty::receiveFromInterface (Packet *packet, NetInterface *interface)
{
	m_entry->recv (packet, 0);
}

void
NodeEmpty::attachAgent (Agent *agent)
{
	Agent *nullAgent = static_cast <Agent *> (TclObject::New ("Agent/Null"));
	agent->dport () = m_demux->allocPort (nullAgent);
	agent->addr () = getAddress ();
	agent->target (m_interfaceConnector);

#if 0
	Node instproc attach { agent { port "" } } {
	$self instvar agents_ address_ dmux_ 
	#
	# Assign port number (i.e., this agent receives
	# traffic addressed to this host and port)
	#
	lappend agents_ $agent
	#
	# Attach agents to this node (i.e., the classifier inside).
	# We call the entry method on ourself to find the front door
	# (e.g., for multicast the first object is not the unicast classifier)
	#
	# Also, stash the node in the agent and set the local addr of 
	# this agent.
	#
	$agent set node_ $self
	$agent set agent_addr_ [AddrParams addr2id $address_]
	#
	# If a port demuxer doesn't exist, create it.
	#
	if { $dmux_ == "" } {
		# Use the default mask_ and port_ values
		set dmux_ [new Classifier/Port]
		# point the node's routing entry to itself
		# at the port demuxer (if there is one)
		$self add-route $address_ $dmux_
	}
	if { $port == "" } {
		set port [$dmux_ alloc-port [[Simulator instance] nullagent]]
	}
	$agent set agent_port_ $port

	$self add-target $agent $port
#endif
}

int 
NodeEmpty::command(int argc, const char*const* argv)
{
	if (argc == 4) {
		if (strcmp (argv[1], "set-var") == 0) {
			if (strcmp (argv[2], "x") == 0) {
				m_x = atof (argv[3]);
			} else if (strcmp (argv[2] , "y") == 0) {
				m_y = atof (argv[3]);
			} else if (strcmp (argv[2] , "z") == 0) {
				m_y = atof (argv[3]);
			} else if (strcmp (argv[2] , "speedX") == 0) {
				m_speedX = atof (argv[3]);
			} else if (strcmp (argv[2] , "speedY") == 0) {
				m_speedY = atof (argv[3]);
			} else if (strcmp (argv[2] , "speedZ") == 0) {
				m_speedZ = atof (argv[3]);
			} else {
				goto out;
			}
			return TCL_OK;
		} else if (strcmp (argv[1], "add-interface") == 0) {
			NetInterfaceConstructor *constructor = static_cast <NetInterfaceConstructor *> (lookup (argv[2]));
			BroadcastChannel *channel = static_cast <BroadcastChannel *> (lookup (argv[3]));
			m_interface = constructor->createInterface ();
			m_interface->connectTo (channel, this);
			return TCL_OK;
		}
	} else if (argc == 3) {
		if (strcmp (argv[1], "attach-agent") == 0) {
			Agent *agent = static_cast <Agent *> (lookup (argv[2]));
			attachAgent (agent);
			return TCL_OK;
		} 
	}
 out:
	printf ("argv: %s/%s/%s \n", argv[0], argv[1], argv[2]);
	return TclObject::command (argc, argv);
}
