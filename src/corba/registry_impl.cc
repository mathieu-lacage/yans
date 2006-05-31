
#include <registry_impl.h>


// Implementation for interface Registry

void
Registry_impl::_cxx_register( ::Echo_ptr obj, const char* name )
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}


::Echo_ptr
Registry_impl::lookup( const char* name )
  throw(
    ::CORBA::SystemException)

{
  ::Echo_ptr retval;

  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

  return retval; 
}

