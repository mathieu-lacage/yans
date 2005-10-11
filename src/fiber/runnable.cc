/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "runnable.h"
#include "fiber-scheduler.h"
#include "fiber.h"


Runnable::Runnable ()
{}
Runnable::~Runnable ()
{}

Host *
Runnable::get_host (void)
{
	Host *host = FiberScheduler::instance ()->get_current ()->get_host ();
	return host;
}

