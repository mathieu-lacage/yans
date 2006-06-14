/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include <cassert>
#include "context-simulator.h"
#include "yans/simulator.h"

ContextSimulator::ContextSimulator ()
	: m_finish (false),
	  m_running (false),
	  m_run (0)
{}

void 
ContextSimulator::stop_at_us (uint64_t at_us)
{
	assert (!m_running);
	yans::Simulator::stop_at_us (at_us);
}
void 
ContextSimulator::start (yans::Callback<void> stopped)
{
	m_stopped = stopped;
	m_run.post ();
}
void 
ContextSimulator::real_run (void)
{
	while (!m_finish) {
		m_run.wait ();
		m_running = true;
		yans::Simulator::run ();
		m_running = false;
		m_stopped ();
	}
}
