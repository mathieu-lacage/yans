#include "stopped-callback-impl.h"

// Implementation for interface StoppedCallback

void 
StoppedCallback_impl::set_callback (yans::Callback<void, bool, uint64_t> cb)
{
  m_cb = cb;
}

void
StoppedCallback_impl::invoke( CORBA::Boolean finished, ::Remote::Timestamp time )
  throw(
    ::CORBA::SystemException)

{
  bool is_finished = finished;
  uint64_t time_us = time;
  
  m_cb (is_finished, time_us);
}
