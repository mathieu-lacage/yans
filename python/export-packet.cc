/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "yans/packet.h"

using namespace boost::python;
using namespace yans;

void export_packet (void)
{
	class_<Packet, boost::shared_ptr<Packet> > ("Packet");
}
