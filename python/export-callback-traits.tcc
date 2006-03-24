/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef EXPORT_CALLBACK_TRAITS_TCC
#define EXPORT_CALLBACK_TRAITS_TCC

#include "yans/packet.h"
#include <boost/shared_ptr.hpp>

template<typename T>
struct ExportCallbackTraits
{
	typedef T type;
	static T make_arg( T a )
	{
		return a;
	}
};

class PacketDeleter {
public:
	void operator () (yans::Packet *packet) {
		packet->unref ();
	}
};

template<>
struct ExportCallbackTraits<yans::Packet*>
{
	typedef boost::shared_ptr<yans::Packet> type;
	static type make_arg(yans::Packet *packet)
	{
		packet->ref ();
		return boost::shared_ptr<yans::Packet> (packet, PacketDeleter ());
	}
};

#endif /* EXPORT_CALLBACK_TRAITS_TCC */
