/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "method-callback.tcc"
#include "yans/pcap-writer.h"
#include "yans/callback.h"
#include "yans/packet.h"

using namespace boost::python;
using namespace yans;

void export_pcap_writer (void)
{
	class_<PcapWriter, boost::noncopyable> writer ("PcapWriter");
	writer.def ("open", &PcapWriter::open);
	writer.def ("write_header_ethernet", &PcapWriter::write_header_ethernet);
	writer.def ("write_packet", &PcapWriter::write_packet);

	EXPORT_METHOD_AS_CALLBACK (PcapWriter,write_packet);
}
