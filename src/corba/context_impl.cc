
#include <context_impl.h>
#include "context-simulator.h"
#include "local-object-registry.h"

namespace {

CORBA::ORB_var g_orb;

PortableServer::POA_var 
get_root_poa (void)
{
  CORBA::Object_var poa_obj = g_orb->resolve_initial_references ("RootPOA");
  PortableServer::POA_var poa = PortableServer::POA::_narrow (poa_obj);
  return poa;
}
void
activate_servant (PortableServer::StaticImplementation *servant)
{
  get_root_poa ()->activate_object (servant);
}
};


// Implementation for interface LocalInstance

LocalInstance_impl::LocalInstance_impl (::Remote::InstanceId id)
  : m_id (id)
{}


::Remote::InstanceId
LocalInstance_impl::get_id()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::InstanceId retval;

  retval = m_id;

  return retval; 
}


// Implementation for interface PositionModel

// Implementation for interface StaticPositionModel

StaticPositionModel_impl::StaticPositionModel_impl (::Remote::InstanceId id)
  : LocalInstance_impl (id),
    m_self (new yans::StaticPosition ())
{}

StaticPositionModel_impl::~StaticPositionModel_impl ()
{
  delete m_self;
}

void
StaticPositionModel_impl::get( CORBA::Double_out x, CORBA::Double_out y, CORBA::Double_out z )
  throw(
    ::CORBA::SystemException)

{
  m_self->get (x,y,z);
}


void
StaticPositionModel_impl::set( CORBA::Double x, CORBA::Double y, CORBA::Double z )
  throw(
    ::CORBA::SystemException)

{
  m_self->set (x,y,z);
}


// Implementation for interface Channel80211

Channel80211_impl::Channel80211_impl (::Remote::InstanceId id, ParallelChannel80211 *real_channel)
  : LocalInstance_impl (id),
    m_self (real_channel)
{}
Channel80211_impl::~Channel80211_impl ()
{
  delete m_self;
}


void
Channel80211_impl::add( ::Remote::Channel80211_ptr channel )
  throw(
    ::CORBA::SystemException)

{
  m_self->add (channel);
}


void
Channel80211_impl::receive(const ::Remote::SourcePosition& source_position, ::Remote::Timestamp source_time, const ::Remote::Buffer& buffer, CORBA::Double tx_power, CORBA::Octet tx_mode, CORBA::Octet stuff )
  throw(
    ::CORBA::SystemException)

{
  m_self->receive (source_position, source_time, buffer, tx_power, tx_mode, stuff);
}


void
Channel80211_impl::receive_null( ::Remote::Timestamp ts, const ::Remote::SourcePositions& sources )
  throw(
    ::CORBA::SystemException)

{
  m_self->receive_null (ts, sources);
}


// Implementation for interface MacNetworkInterface

// Implementation for interface NetworkInterface80211

NetworkInterface80211_impl::NetworkInterface80211_impl (::Remote::InstanceId id, NetworkInterface80211 *real_interface)
  : LocalInstance_impl (id),
    m_self (real_interface)
{}

NetworkInterface_impl::~NetworkInterface_impl ()
{
  delete m_self;
}

void
NetworkInterface80211_impl::connect( ::Remote::Channel80211_ptr channel )
  throw(
    ::CORBA::SystemException)

{
  yans::BaseChannel80211 *real_channel = LocalObjectRegistry::lookup_channel_80211 (channel);
  assert (real_channel != 0);
  m_self->connect (real_channel);
}


// Implementation for interface NetworkInterface80211Factory

NetworkInterface80211Factory_impl::NetworkInterface80211Factory_impl (yans::NetworkInterface80211Factory *real_factory)
  : m_self (real_factory)
{}

NetworkInterface80211Factory_impl::~NetworkInterface80211Factory_impl ()
{
  delete m_self;
}

::Remote::NetworkInterface80211_ptr
NetworkInterface80211Factory_impl::create_adhoc( const ::Remote::MacAddress& address, ::Remote::PositionModel_ptr position, ::Remote::InstanceId id )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::NetworkInterface80211_ptr retval;

  Position *real_position_model = LocalObjectRegistry::lookup_static_position (position->get_id ());
  NetworkInterface80211 *real_interface = m_self->create_adhoc (address, real_position_model);
  LocalObjectRegistry::record_mac_interface (id, real_interface);
  NetworkInterface80211_impl *servant = new NetworkInterface80211_impl (id, real_interface);
  activate_servant (servant);
  retval = servant->_this ();

  return retval; 
}


// Implementation for interface Ipv4NetworkInterface

/* We do not hold a pointer to the real ipv4 interface because we do not invoke any
 * method on this object. We just use it as a placeholder for the instance id
 * which is used later to retrieve the yans::Ipv4NetworkInterface when needed.
 */
Ipv4NetworkInterface_impl::Ipv4NetworkInterface_impl (::Remote::InstanceId id)
  : LocalInstance (id)
{}
Ipv4NetworkInterface_impl::~Ipv4NetworkInterface_impl ()
{}


// Implementation for interface Ipv4RoutingTable

Ipv4RoutingTable_impl::Ipv4RoutingTable_impl (yans::Ipv4Route *real_table)
  : m_self (real_table)
{}

Ipv4RoutingTable_impl::~Ipv4RoutingTable_impl ()
{
  // no need to delete routing table. it is a pointer
  // to a data structure maintained within the Node/Host
}

void
Ipv4RoutingTable_impl::set_default_route( ::Remote::Ipv4Address next_hop, ::Remote::Ipv4NetworkInterface_ptr i )
  throw(
    ::CORBA::SystemException)

{
  yans::Ipv4NetworkInterface *real_interface = LocalObjectRegistry::lookup_ipv4_interface (i->get_id ());
  yans::Ipv4Address real_next_hop;
  real_next_hop->set_host_order (next_hop);
  m_self->set_default_route (real_next_hop, real_interface);
}


// Implementation for interface Node

Node_impl::Node_impl (::Remote::InstanceId id, yans::Host *real_node)
  : LocalInstance_impl (id),
    m_self (host)
{
  m_routing_table = new Ipv4RoutingTable_impl (m_self->get_routing_table ());
  activate_servant (m_routing_table);
}

Node_impl::~Node_impl ()
{
  delete m_self;
  delete m_routing_table;
  for (Ipv4InterfacesI i = m_interfaces.begin (); i != m_interfaces.end (); i++) {
    delete (*i);
  }
  m_interfaces.erase (m_interfaces.begin (), m_interface.end ());
}

::Remote::Ipv4RoutingTable_ptr
Node_impl::get_routing_table()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Ipv4RoutingTable_ptr retval;

  retval = m_routing_table->_this ();

  return retval; 
}


::Remote::Ipv4NetworkInterface_ptr
Node_impl::add_ipv4_arp_interface( ::Remote::MacNetworkInterface_ptr i, ::Remote::Ipv4Address address, ::Remote::Ipv4Mask mask, ::Remote::InstanceId id )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Ipv4NetworkInterface_ptr retval;

  yans::MacNetworkInterface *real_interface = LocalObjectRegistry::lookup_mac_interface (i->get_id ());
  yans::Ipv4Address real_address;
  yans::Ipv4Mask real_mask;
  real_address.set_host_order (address);
  real_mark.set_host_order (real_mask);
  yans::Ipv4NetworkInterface *real_ipv4_interface = m_self->add_ipv4_arp_interface (real_interface, real_address, real_mask);
  Ipv4NetworkInterface_impl *servant = new Ipv4NetworkInterface_impl (id, real_ipv4_interface);
  m_interfaces.add (servant);
  activate_servant (servant);
  retval = servant->_this ();

  return retval; 
}


// Implementation for interface CallbackVoidPacket

/* 
 * Same here, This class is use donly as a placeholder for the
 *  instance id which is used to retrieve the pointer to a
 * yans::Callback<void, Packet*> when needed.
 */
CallbackVoidPacket_impl (::Remote::InstanceId id)
  : LocalInstance_impl (id)
{}

// Implementation for interface UdpSource

UdpSource_impl::UdpSource_impl (yans::UdpSource *real_source)
  : m_self (real_source)
{}

UdpSource_impl::~UdpSource_impl ()
{
  delete m_self;
}

void
UdpSource_impl::bind( ::Remote::Ipv4Address address, CORBA::UShort port )
  throw(
    ::CORBA::SystemException)

{
  yans::Ipv4Address real_address;
  real_address.set_host_order (address);
  m_self->bind (real_address, port);
}


void
UdpSource_impl::set_peer( ::Remote::Ipv4Address remote, CORBA::UShort port )
  throw(
    ::CORBA::SystemException)

{
  yans::Ipv4Address real_address;
  real_address.set_host_order (remote);
  m_self->set_peer (real_address, port);
}


void
UdpSource_impl::unbind_at_s( CORBA::Double at_s )
  throw(
    ::CORBA::SystemException)

{
  m_self->unbind_at_s (at_s);
}


::Remote::CallbackVoidPacket_ptr
UdpSource_impl::create_send_callback( ::Remote::InstanceId id )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::CallbackVoidPacket_ptr retval;

  Callback<void,Packet *> real_cb = make_callback (&UdpSource::send, m_self);
  LocalObjectRegistry::record_callback_void_packet (id, real_cb);
  CallbackVoidPacket_impl *servant = new CallbackVoidPacket_impl (id);
  activate_servant (servant);
  retval = servant->_this ();

  return retval; 
}


// Implementation for interface UdpSink

UdpSink_impl::UdpSink_impl (yans::UdpSink *real_sink)
  : m_self (real_sink)
{}

UdpSink_impl::~UdpSink_impl ()
{
  delete m_self;
}

void
UdpSink_impl::bind( ::Remote::Ipv4Address address, CORBA::UShort port )
  throw(
    ::CORBA::SystemException)

{
  yans::Ipv4Address real_address;
  real_address.set_host_order (address);
  m_self->bind (real_address, port);
}


void
UdpSink_impl::unbind_at_s( CORBA::Double at_s )
  throw(
    ::CORBA::SystemException)

{
  m_self->unbind_at_s (at_s);
}


void
UdpSink_impl::set_receive_callback( ::Remote::CallbackVoidPacket_ptr callback )
  throw(
    ::CORBA::SystemException)

{
  yans::Callback<void,Packet *> real_cb = LocalObjectRegistry::lookup_callback_void_packet (callback->get_id ());
  m_self->set_receive_callback (real_cb);
}


// Implementation for interface PeriodicGenerator

PeriodicGenerator_impl::PeriodicGenerator_impl (yans::PeriodicGenerator *real_generator)
  : m_self (real_generator)
{}

PeriodicGenerator_impl::~PeriodicGenerator_impl ()
{
  delete m_self;
}

void
PeriodicGenerator_impl::set_packet_interval_s( CORBA::Double interval_s )
  throw(
    ::CORBA::SystemException)

{
  m_self->set_packet_interval_s (interval_s);
}


void
PeriodicGenerator_impl::set_packet_size( CORBA::ULong size )
  throw(
    ::CORBA::SystemException)

{
  m_self->set_packet_size (size);
}


void
PeriodicGenerator_impl::set_send_callback( ::Remote::CallbackVoidPacket_ptr callback )
  throw(
    ::CORBA::SystemException)

{
  yans::Callback<void,Packet *> real_cb = LocalObjectRegistry::lookup_callback_void_packet (callback->get_id ());
  m_self->set_send_callback (real_cb);  
}


// Implementation for interface TrafficAnalyser

TrafficAnalyser_impl::TrafficAnalyser_impl (yans::TrafficAnalyser *analyser)
  : m_self (analyser)
{}

TrafficAnalyser_impl::~TrafficAnalyser_impl ()
{
  delete m_self;
}

::Remote::CallbackVoidPacket_ptr
TrafficAnalyser_impl::create_receive_callback( ::Remote::InstanceId id )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::CallbackVoidPacket_ptr retval;

  yans::Callback<void,Packet *> real_cb = yans::make_callback (&yans::TrafficAnalyser::receive, m_self);
  CallbackVoidPacket_impl *servant = new CallbackVoidPacket_impl (id);
  activate_servant (servant);
  retval = servant->_this ();

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

ComputingContext_impl::ComputingContext_impl ()
  : m_simulator (new ContextSimulator ())
{}

ComputingContext_impl::~ComputingContext_impl ()
{
  delete m_simulator;
}

::Remote::Node_ptr
ComputingContext_impl::create_node( ::Remote::InstanceId id, const char *name)
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Node_ptr retval;

  yans::Host *real_node = new Host (name);
  Node_impl *servant = new Node_impl (id, real_node);
  activate_servant (servant);
  retval = servant->_this ();

  return retval; 
}


::Remote::NetworkInterface80211Factory_ptr
ComputingContext_impl::create_network_interface_80211_factory()
  throw(
    ::CORBA::SystemException)

{
  ::Remote::NetworkInterface80211Factory_ptr retval;

  NetworkInterface80211Factory *real_factory = new NetworkInterface80211Factory ();
  NetworkInterface80211Factory_impl *servant = new NetworkInterface80211Factory_impl (real_factory);
  activate_servant (servant);
  retval = servant->_this ();

  return retval; 
}


::Remote::Channel80211_ptr
ComputingContext_impl::create_channel_80211( ::Remote::InstanceId id )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::Channel80211_ptr retval;

  ParallelChannel80211 *real_channel = new ParallelChannel80211 ();
  Channel80211_impl *servant = new Channel80211_impl (id, real_channel);
  activate_servant (servant);
  retval = servant->_this ();

  return retval; 
}


::Remote::StaticPositionModel_ptr
ComputingContext_impl::create_static_position( ::Remote::InstanceId id )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::StaticPositionModel_ptr retval;

  StaticPositionModel_impl *servant = new StaticPositionModel_impl ();
  LocalObjectRegistry::instance ()->get_poa ()->activate_object (servant);
  retval = servant->_this ();
  LocalObjectRegistry->record (retval, servant);

  return retval; 
}


::Remote::UdpSource_ptr
ComputingContext_impl::create_udp_source( ::Remote::Node_ptr node )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::UdpSource_ptr retval;

  UdpSource_impl *servant = new UdpSource_impl ();
  LocalObjectRegistry::instance ()->get_poa ()->activate_object (servant);
  retval = servant->_this ();

  return retval; 
}


::Remote::UdpSink_ptr
ComputingContext_impl::create_udp_sink( ::Remote::Node_ptr node )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::UdpSink_ptr retval;

  UdpSink_impl *servant = new UdpSink_impl ();
  LocalObjectRegistry::instance ()->get_poa ()->activate_object (servant);
  retval = servant->_this ();
  
  return retval; 
}


void
ComputingContext_impl::stop_at_us( ::Remote::Timestamp at_us )
  throw(
    ::CORBA::SystemException)

{
  m_simulator->stop_at_us (at_us);
}

void
ComputingContext_impl::run_done (void)
{
  if (!CORBA::is_nil (m_run_done)) {
    m_run_done->invoke ();
  }
}

void
ComputingContext_impl::start( ::Remote::CallbackVoid_ptr done )
  throw(
    ::CORBA::SystemException)

{
  if (!CORBA::is_nil (m_run_done)) {
    CORBA::release (m_run_done);
  }
  m_run_done = ::Remote::CallbackVoid::_duplicate (done);
  m_simulator->start (yans::make_callback (&ComputingContext_impl::run_done, this));
}

void
ComputingContext_impl::shutdown()
  throw(
    ::CORBA::SystemException)

{
  LocalObjectRegistry::instance ()->get_orb ()->shutdown (0);
}

