
#ifndef __CONTEXT_IMPL_H__
#define __CONTEXT_IMPL_H__

#include <remote-context.h>

namespace yans {
  class UdpSink;
  class UdpSource;
  class PeriodicGenerator;
  class StaticPosition;
  class TrafficAnalyser;
  class Host;
  class NetworkInterface80211;
  class NetworkInterface80211Adhoc;
  class NetworkInterface80211Factory;
  class Ipv4Route;
};

class ParallelChannel80211;

void impl_set_orb (CORBA::ORB_var orb);


// Implementation for interface LocalInstance
class LocalInstance_impl : virtual public POA_Remote::LocalInstance
{
  public:
  LocalInstance_impl (::Remote::InstanceId id);

    ::Remote::InstanceId get_id()
      throw(
        ::CORBA::SystemException)
    ;
 private:
  LocalInstance_impl ();
  ::Remote::InstanceId m_id;
};


// Implementation for interface PositionModel
class PositionModel_impl : 
  virtual public LocalInstance_impl,
  virtual public POA_Remote::PositionModel
{
  public:
  PositionModel_impl (::Remote::InstanceId id);
};


// Implementation for interface StaticPositionModel
class StaticPositionModel_impl : 
  virtual public PositionModel_impl,
  virtual public POA_Remote::StaticPositionModel
{
  public:
  StaticPositionModel_impl (::Remote::InstanceId id, yans::StaticPosition *real_position);
  ~StaticPositionModel_impl ();

    void get( CORBA::Double_out x, CORBA::Double_out y, CORBA::Double_out z )
      throw(
        ::CORBA::SystemException)
    ;

    void set( CORBA::Double x, CORBA::Double y, CORBA::Double z )
      throw(
        ::CORBA::SystemException)
    ;
 private:
    yans::StaticPosition *m_self;
};


// Implementation for interface Channel80211
class Channel80211_impl : 
  virtual public LocalInstance_impl,
  virtual public POA_Remote::Channel80211
{
  public:
  Channel80211_impl (::Remote::InstanceId id, ParallelChannel80211 *real_channel);
  ~Channel80211_impl ();
  
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
 private:
    ParallelChannel80211 *m_self;
};


// Implementation for interface MacNetworkInterface
class MacNetworkInterface_impl : 
  virtual public LocalInstance_impl,
  virtual public POA_Remote::MacNetworkInterface
{
  public:
  MacNetworkInterface_impl (::Remote::InstanceId id);
};


// Implementation for interface NetworkInterface80211
class NetworkInterface80211_impl : 
  virtual public MacNetworkInterface_impl,
  virtual public POA_Remote::NetworkInterface80211
{
  public:
  NetworkInterface80211_impl (::Remote::InstanceId id, yans::NetworkInterface80211 *real_interface);
  ~NetworkInterface80211_impl ();

    void connect( ::Remote::Channel80211_ptr channel )
      throw(
        ::CORBA::SystemException)
    ;

 private:
    yans::NetworkInterface80211 *m_self;
};

class NetworkInterface80211Adhoc_impl : 
virtual public NetworkInterface80211_impl,
  virtual public POA_Remote::NetworkInterface80211Adhoc
{
 public:
  NetworkInterface80211Adhoc_impl (::Remote::InstanceId id, 
				   yans::NetworkInterface80211Adhoc *real_interface);
  virtual ~NetworkInterface80211Adhoc_impl ();
};


// Implementation for interface NetworkInterface80211Factory
class NetworkInterface80211Factory_impl : virtual public POA_Remote::NetworkInterface80211Factory
{
  public:
  NetworkInterface80211Factory_impl  (yans::NetworkInterface80211Factory *real_factory);
  ~NetworkInterface80211Factory_impl ();

    ::Remote::NetworkInterface80211Adhoc_ptr create_adhoc( const ::Remote::MacAddress& address, ::Remote::PositionModel_ptr position, ::Remote::InstanceId id )
      throw(
        ::CORBA::SystemException)
    ;
 private:
  yans::NetworkInterface80211Factory *m_self;
};


// Implementation for interface Ipv4NetworkInterface
class Ipv4NetworkInterface_impl : 
  virtual public LocalInstance_impl,
  virtual public POA_Remote::Ipv4NetworkInterface
{
  public:
  Ipv4NetworkInterface_impl (::Remote::InstanceId id);
  ~Ipv4NetworkInterface_impl ();
};


// Implementation for interface Ipv4RoutingTable
class Ipv4RoutingTable_impl : virtual public POA_Remote::Ipv4RoutingTable
{
  public:
  Ipv4RoutingTable_impl (yans::Ipv4Route *real_routing_table);
  ~Ipv4RoutingTable_impl ();

    void set_default_route( ::Remote::Ipv4Address next_hop, ::Remote::Ipv4NetworkInterface_ptr i )
      throw(
        ::CORBA::SystemException)
    ;
 private:
    yans::Ipv4Route *m_self;
};


// Implementation for interface Node
class Node_impl : 
  virtual public LocalInstance_impl,
  virtual public POA_Remote::Node
{
  public:
  Node_impl (::Remote::InstanceId id, yans::Host *real_node);
  ~Node_impl ();

    ::Remote::Ipv4RoutingTable_ptr get_routing_table()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::Ipv4NetworkInterface_ptr add_ipv4_arp_interface( ::Remote::MacNetworkInterface_ptr i, ::Remote::Ipv4Address address, ::Remote::Ipv4Mask mask, ::Remote::InstanceId id )
      throw(
        ::CORBA::SystemException)
    ;
 private:
  yans::Host *m_self;
  typedef std::vector<Ipv4NetworkInterface_impl *> Ipv4Interfaces;
  typedef std::vector<Ipv4NetworkInterface_impl *>::iterator Ipv4InterfacesI;
  Ipv4Interfaces m_interfaces;
  Ipv4RoutingTable_impl *m_routing_table;
};


// Implementation for interface CallbackVoidPacket
class CallbackVoidPacket_impl : 
  virtual public LocalInstance_impl,
  virtual public POA_Remote::CallbackVoidPacket
{
  public:
  CallbackVoidPacket_impl (::Remote::InstanceId id);
};


// Implementation for interface UdpSource
class UdpSource_impl : virtual public POA_Remote::UdpSource
{
  public:
  UdpSource_impl (yans::UdpSource *real_source);
  ~UdpSource_impl ();

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

    ::Remote::CallbackVoidPacket_ptr create_send_callback( ::Remote::InstanceId id )
      throw(
        ::CORBA::SystemException)
    ;
 private:
    yans::UdpSource *m_self;
};


// Implementation for interface UdpSink
class UdpSink_impl : virtual public POA_Remote::UdpSink
{
  public:
  UdpSink_impl (yans::UdpSink *real_sink);
  ~UdpSink_impl ();

    void bind( ::Remote::Ipv4Address address, CORBA::UShort port )
      throw(
        ::CORBA::SystemException)
    ;

    void unbind_at_s( CORBA::Double at_s )
      throw(
        ::CORBA::SystemException)
    ;

    void set_receive_callback( ::Remote::CallbackVoidPacket_ptr callback )
      throw(
        ::CORBA::SystemException)
    ;
 private:
    yans::UdpSink *m_self;
};


// Implementation for interface PeriodicGenerator
class PeriodicGenerator_impl : virtual public POA_Remote::PeriodicGenerator
{
  public:
  PeriodicGenerator_impl (yans::PeriodicGenerator *real_generator);
  ~PeriodicGenerator_impl ();

    void set_packet_interval_s( CORBA::Double interval_s )
      throw(
        ::CORBA::SystemException)
    ;

    void set_packet_size( CORBA::ULong size )
      throw(
        ::CORBA::SystemException)
    ;

    void set_send_callback( ::Remote::CallbackVoidPacket_ptr callback )
      throw(
        ::CORBA::SystemException)
    ;
 private:
    yans::PeriodicGenerator *m_self;
};


// Implementation for interface TrafficAnalyser
class TrafficAnalyser_impl : virtual public POA_Remote::TrafficAnalyser
{
  public:
  TrafficAnalyser_impl (yans::TrafficAnalyser *real_analyser);
  ~TrafficAnalyser_impl ();

    ::Remote::CallbackVoidPacket_ptr create_receive_callback( ::Remote::InstanceId id )
      throw(
        ::CORBA::SystemException)
    ;
 private:
  yans::TrafficAnalyser *m_self;
};



// Implementation for interface ComputingContext
class ComputingContext_impl : virtual public POA_Remote::ComputingContext
{
  public:
  ComputingContext_impl ();
  ~ComputingContext_impl ();

  ::Remote::Node_ptr create_node( ::Remote::InstanceId id, const char *name)
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::NetworkInterface80211Factory_ptr create_network_interface_80211_factory()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::Channel80211_ptr create_channel_80211( ::Remote::InstanceId id )
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::StaticPositionModel_ptr create_static_position( ::Remote::InstanceId id )
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::UdpSource_ptr create_udp_source( ::Remote::Node_ptr node )
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::UdpSink_ptr create_udp_sink( ::Remote::Node_ptr node )
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::PeriodicGenerator_ptr create_periodic_generator()
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::TrafficAnalyser_ptr create_traffic_analyser()
      throw(
        ::CORBA::SystemException)
    ;

    void stop_at_us( ::Remote::Timestamp at_us )
      throw(
        ::CORBA::SystemException)
    ;

    void start( ::Remote::CallbackVoid_ptr done )
      throw(
        ::CORBA::SystemException)
    ;

    void shutdown()
      throw(
        ::CORBA::SystemException)
    ;
 private:
    void run_done (void);
    class ContextSimulator *m_simulator;
    ::Remote::CallbackVoid_ptr m_run_done;
};


#endif
