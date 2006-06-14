#ifndef STOPPED_CALLBACK_IMPL_H
#define STOPPED_CALLBACK_IMPL_H

#include "remote-context.h"
#include "yans/callback.h"

// Implementation for interface StoppedCallback
class StoppedCallback_impl : virtual public POA_Remote::StoppedCallback
{
  public:
  void set_callback (yans::Callback<void, bool, uint64_t> cb);

    void invoke( CORBA::Boolean finished, ::Remote::Timestamp time )
      throw(
        ::CORBA::SystemException)
    ;
 private: 
    yans::Callback<void, bool, uint64_t> m_cb;
};

#endif /* STOPPED_CALLBACK_IMPL_H */
