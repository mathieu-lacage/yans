/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <boost/python.hpp>
#include "pcap-writer.h"
#include "export-callback.tcc"
#include "callback.tcc"
#include "packet.h"

using namespace boost::python;
using namespace yans;

void export_pcap_writer (void)
{
	class_<PcapWriter, boost::noncopyable> writer ("PcapWriter");
	writer.def ("open", &PcapWriter::open);
	writer.def ("write_header_ethernet", &PcapWriter::write_header_ethernet);
	writer.def ("write_packet", &PcapWriter::write_packet);

	export_method_as_callback<struct foo> ("PcapWriter_write_packet_callback",
					       &PcapWriter::write_packet);
}
