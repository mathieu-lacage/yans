
#include <registry_impl.h>
#include <iostream>

// Implementation for interface Registry

Registry_impl::Registry_impl (CORBA::ORB_var orb)
  : m_orb (orb)
{}

void
Registry_impl::set_callback (yans::Callback<void> done)
{
  m_done = done;
}

void
Registry_impl::record( ::Remote::ComputingContext_ptr obj, const char* name )
  throw(
    ::CORBA::SystemException)

{
  m_contexts.push_back (std::make_pair (name, ::Remote::ComputingContext::_duplicate (obj)));
  m_done ();
}


::Remote::ComputingContext_ptr
Registry_impl::lookup( const char* name )
  throw(
    ::CORBA::SystemException)

{
  ::Remote::ComputingContext_ptr retval = ::Remote::ComputingContext::_nil ();

  for (ContextsI i = m_contexts.begin (); i != m_contexts.end (); i++) {
    if (i->first.compare (name)) {
      std::cout << "lookup ok" << std::endl;
      retval = ::Remote::ComputingContext::_duplicate (i->second);
      break;
    }
  }

  return retval;
}


void 
Registry_impl::shutdown_recorded (void)
      throw(
        ::CORBA::SystemException)

{
  for (ContextsI i = m_contexts.begin (); i != m_contexts.end (); i++) {
    i->second->shutdown ();
  }
}
void 
Registry_impl::shutdown (void)
      throw(
        ::CORBA::SystemException)
{
  m_orb->shutdown (false);
}
