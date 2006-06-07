
#ifndef __REGISTRY_IMPL_H__
#define __REGISTRY_IMPL_H__

#include "yans/callback.h"
#include "id-factory.h"
#include <registry.h>
#include <list>
#include <utility>


// Implementation for interface Registry
class Registry_impl : virtual public POA_Registry
{
  public:
  Registry_impl (CORBA::ORB_var orb);
  void set_callback (yans::Callback<void> done);

  void record( ::Remote::ComputingContext_ptr obj, const char* name )
      throw(
        ::CORBA::SystemException)
    ;

    ::Remote::ComputingContext_ptr lookup( const char* name )
      throw(
        ::CORBA::SystemException)
    ;

  ::Registry::Id get_id(void)
      throw(
	    ::CORBA::SystemException)
      ;
    void shutdown_recorded (void)
      throw(
        ::CORBA::SystemException)
    ;
    void shutdown (void)
      throw(
        ::CORBA::SystemException)
    ;
 private:
    typedef std::list<std::pair<std::string, ::Remote::ComputingContext_var> > Contexts;
    typedef std::list<std::pair<std::string, ::Remote::ComputingContext_var> >::iterator ContextsI;
    Contexts m_contexts;
    CORBA::ORB_var m_orb;
    yans::Callback<void> m_done;
    IdFactory m_ids;
};


#endif
