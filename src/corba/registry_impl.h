
#ifndef __REGISTRY_IMPL_H__
#define __REGISTRY_IMPL_H__

#include <registry.h>


// Implementation for interface Registry
class Registry_impl : virtual public POA_Registry
{
  public:

    void _cxx_register( ::Echo_ptr obj, const char* name )
      throw(
        ::CORBA::SystemException)
    ;

    ::Echo_ptr lookup( const char* name )
      throw(
        ::CORBA::SystemException)
    ;
};


#endif
