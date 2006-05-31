
#include <echo_impl.h>


// Implementation for interface Echo

void
Echo_impl::print()
  throw(
    ::CORBA::SystemException)

{
  // add your implementation here
    // REMOVE  
    mico_throw(::CORBA::NO_IMPLEMENT());
    // REMOVE 

}

