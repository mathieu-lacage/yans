
#ifndef __CONTEXT_IMPL_H__
#define __CONTEXT_IMPL_H__

#include <context.h>


// Implementation for interface PositionModel
class PositionModel_impl : virtual public POA_Remote::PositionModel
{
  public:
};


// Implementation for interface Channel80211
class Channel80211_impl : virtual public POA_Remote::Channel80211
{
  public:

    void add( ::Remote::Channel80211_ptr channel )
      throw(
        ::CORBA::SystemException)
    ;

    void receive( const ::Remote::SourcePosition& source_position, ::Remote::Timestamp source_time, const ::Remote::Buffer& buffer, CORBA::Double tx_power, CORBA::Octet tx_mode, CORBA::Octet stuff )
      throw(
        ::CORBA::SystemException)
    ;

    void receive_null( ::Remote::Timestamp ts, const ::Remote::SourcePositions& sources )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface MacNetworkInterface
class MacNetworkInterface_impl : virtual public POA_Remote::MacNetworkInterface
{
  public:
};


// Implementation for interface NetworkInterface80211
class NetworkInterface80211_impl : 
  virtual public MacNetworkInterface_impl,
  virtual public POA_Remote::NetworkInterface80211
{
  public:

    void connect( ::Remote::Channel80211_ptr channel )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface NetworkInterface80211Factory
class NetworkInterface80211Factory_impl : virtual public POA_Remote::NetworkInterface80211Factory
{
  public:

    ::Remote::NetworkInterface80211_ptr create_adhoc( const ::Remote::MacAddress& address, ::Remote::PositionModel_ptr position )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface Ipv4NetworkInterface
class Ipv4NetworkInterface_impl : virtual public POA_Remote::Ipv4NetworkInterface
{
  public:
};


// Implementation for interface Ipv4RoutingTable
class Ipv4RoutingTable_impl : virtual public POA_Remote::Ipv4RoutingTable
{
  public:

    void set_default_route( ::Remote::Ipv4Address next_hop, ::Remote::Ipv4NetworkInterface_ptr i )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface Node
class Node_impl : virtual public POA_Remote::Node
{
  public:

    ::Remote::Ipv4RoutingTable_ptr get_routing_table()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::Ipv4NetworkInterface_ptr add_ipv4_arp_interface( ::Remote::MacNetworkInterface_ptr i, ::Remote::Ipv4Address address, ::Remote::Ipv4Mask mask )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface LocalCallbackVoidPacket
class LocalCallbackVoidPacket_impl : virtual public POA_Remote::LocalCallbackVoidPacket
{
  public:
};


// Implementation for interface UdpSource
class UdpSource_impl : virtual public POA_Remote::UdpSource
{
  public:

    void bind( ::Remote::Ipv4Address address, CORBA::UShort port )
      throw(
        ::CORBA::SystemException)
    ;

    void set_peer( ::Remote::Ipv4Address remote, CORBA::UShort port )
      throw(
        ::CORBA::SystemException)
    ;

    void unbind_at_s( CORBA::Double at_s )
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::LocalCallbackVoidPacket_ptr create_send_callback()
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface UdpSink
class UdpSink_impl : virtual public POA_Remote::UdpSink
{
  public:

    void bind( ::Remote::Ipv4Address address, CORBA::UShort port )
      throw(
        ::CORBA::SystemException)
    ;

    void unbind_at_s( CORBA::Double at_s )
      throw(
        ::CORBA::SystemException)
    ;

    void set_receive_callback( ::Remote::LocalCallbackVoidPacket_ptr callback )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface PeriodicGenerator
class PeriodicGenerator_impl : virtual public POA_Remote::PeriodicGenerator
{
  public:

    void set_packet_interval_s( CORBA::Double interval_s )
      throw(
        ::CORBA::SystemException)
    ;

    void set_packet_size( CORBA::ULong size )
      throw(
        ::CORBA::SystemException)
    ;

    void set_send_callback( ::Remote::LocalCallbackVoidPacket_ptr callback )
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface TrafficAnalyser
class TrafficAnalyser_impl : virtual public POA_Remote::TrafficAnalyser
{
  public:

    ::Remote::LocalCallbackVoidPacket_ptr create_receive_callback()
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface CallbackVoid
class CallbackVoid_impl : virtual public POA_Remote::CallbackVoid
{
  public:

    void invoke()
      throw(
        ::CORBA::SystemException)
    ;
};


// Implementation for interface ComputingContext
class ComputingContext_impl : virtual public POA_Remote::ComputingContext
{
  public:
  ComputingContext_impl (CORBA::ORB_var orb);

    ::Remote::Node_ptr create_node()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::NetworkInterface80211Factory_ptr create_network_interface_80211_factory()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::Channel80211_ptr create_channel_80211()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::PositionModel_ptr create_static_position()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::UdpSource_ptr create_udp_source()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::UdpSink_ptr create_udp_sink()
      throw(
        ::CORBA::SystemException)
    ;

    void stop_at_us( ::Remote::CallbackVoid_ptr frozen, ::Remote::Timestamp at_us )
      throw(
        ::CORBA::SystemException)
    ;

    void run( ::Remote::CallbackVoid_ptr done )
      throw(
        ::CORBA::SystemException)
    ;

    void shutdown()
      throw(
        ::CORBA::SystemException)
    ;
 private:
    CORBA::ORB_var m_orb;
};


#endif
