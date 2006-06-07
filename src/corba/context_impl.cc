
#include <context_impl.h>


// Implementation for interface PositionModel

// Implementation for interface Channel80211

void
Channel80211_impl::add( ::Remote::Channel80211_ptr channel )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
Channel80211_impl::receive( const ::Remote::SourcePosition& source_position, ::Remote::Timestamp source_time, const ::Remote::Buffer& buffer, CORBA::Double tx_power, CORBA::Octet tx_mode, CORBA::Octet stuff )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
Channel80211_impl::receive_null( ::Remote::Timestamp ts, const ::Remote::SourcePositions& sources )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


// Implementation for interface MacNetworkInterface

// Implementation for interface NetworkInterface80211

void
NetworkInterface80211_impl::connect( ::Remote::Channel80211_ptr channel )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


// Implementation for interface NetworkInterface80211Factory

::Remote::NetworkInterface80211_ptr
NetworkInterface80211Factory_impl::create_adhoc( const ::Remote::MacAddress& address, ::Remote::PositionModel_ptr position )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::NetworkInterface80211_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


// Implementation for interface Ipv4NetworkInterface

// Implementation for interface Ipv4RoutingTable

void
Ipv4RoutingTable_impl::set_default_route( ::Remote::Ipv4Address next_hop, ::Remote::Ipv4NetworkInterface_ptr i )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


// Implementation for interface Node

::Remote::Ipv4RoutingTable_ptr
Node_impl::get_routing_table()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Ipv4RoutingTable_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


::Remote::Ipv4NetworkInterface_ptr
Node_impl::add_ipv4_arp_interface( ::Remote::MacNetworkInterface_ptr i, ::Remote::Ipv4Address address, ::Remote::Ipv4Mask mask )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Ipv4NetworkInterface_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


// Implementation for interface LocalCallbackVoidPacket

// Implementation for interface UdpSource

void
UdpSource_impl::bind( ::Remote::Ipv4Address address, CORBA::UShort port )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
UdpSource_impl::set_peer( ::Remote::Ipv4Address remote, CORBA::UShort port )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
UdpSource_impl::unbind_at_s( CORBA::Double at_s )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


::Remote::LocalCallbackVoidPacket_ptr
UdpSource_impl::create_send_callback()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::LocalCallbackVoidPacket_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


// Implementation for interface UdpSink

void
UdpSink_impl::bind( ::Remote::Ipv4Address address, CORBA::UShort port )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
UdpSink_impl::unbind_at_s( CORBA::Double at_s )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
UdpSink_impl::set_receive_callback( ::Remote::LocalCallbackVoidPacket_ptr callback )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


// Implementation for interface PeriodicGenerator

void
PeriodicGenerator_impl::set_packet_interval_s( CORBA::Double interval_s )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
PeriodicGenerator_impl::set_packet_size( CORBA::ULong size )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
PeriodicGenerator_impl::set_send_callback( ::Remote::LocalCallbackVoidPacket_ptr callback )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


// Implementation for interface TrafficAnalyser

::Remote::LocalCallbackVoidPacket_ptr
TrafficAnalyser_impl::create_receive_callback()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::LocalCallbackVoidPacket_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


// Implementation for interface CallbackVoid

void
CallbackVoid_impl::invoke()
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


// Implementation for interface ComputingContext

ComputingContext_impl::ComputingContext_impl (CORBA::ORB_var orb)
  : m_orb (orb)
{}

::Remote::Node_ptr
ComputingContext_impl::create_node()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Node_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


::Remote::NetworkInterface80211Factory_ptr
ComputingContext_impl::create_network_interface_80211_factory()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::NetworkInterface80211Factory_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


::Remote::Channel80211_ptr
ComputingContext_impl::create_channel_80211()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Channel80211_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


::Remote::PositionModel_ptr
ComputingContext_impl::create_static_position()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::PositionModel_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


::Remote::UdpSource_ptr
ComputingContext_impl::create_udp_source()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::UdpSource_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


::Remote::UdpSink_ptr
ComputingContext_impl::create_udp_sink()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::UdpSink_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}


void
ComputingContext_impl::stop_at_us( ::Remote::CallbackVoid_ptr frozen, ::Remote::Timestamp at_us )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
ComputingContext_impl::run( ::Remote::CallbackVoid_ptr done )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


void
ComputingContext_impl::shutdown()
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}

