#ifndef CALLBACK_RUN_DONE_IMPL_H
#define CALLBACK_RUN_DONE_IMPL_H

#include <context.h>

class CallbackVoid_impl : virtual public POA_Remote::CallbackVoid
{
  public:

    void invoke()
      throw(
        ::CORBA::SystemException)
    ;
};

#endif /* CALLBACK_RUN_DONE_IMPL_H */
