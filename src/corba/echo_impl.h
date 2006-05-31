
#ifndef __ECHO_IMPL_H__
#define __ECHO_IMPL_H__

#include <echo.h>


// Implementation for interface Echo
class Echo_impl : virtual public POA_Echo
{
  public:

    void print()
      throw(
        ::CORBA::SystemException)
    ;
};


#endif
