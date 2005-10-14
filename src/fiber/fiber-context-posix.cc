/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-context.h"

#include <setjmp.h>
#include <cassert>

#define FIBER_CONTEXT_TRACE 1

#ifdef FIBER_CONTEXT_TRACE
#include <stdio.h>
# define TRACE(x, b) \
printf ("CONTEXT 0x%lx " x, (long)this, b);
#else /* FIBER_CONTEXT_TRACE */
# define TRACE(x, b)
#endif /* FIBER_CONTEXT_TRACE*/

class FiberContextPrivate {
public:
	jmp_buf m_context;
};

FiberContext::FiberContext ()
{
	m_priv = new FiberContextPrivate ();
}
FiberContext::~FiberContext ()
{
	delete m_priv;
}
void 
FiberContext::save (void)
{
	//uint8_t stack;
	if (setjmp (m_priv->m_context) == 1) {
		//TRACE ("return to 0x%lx\n", (long)&stack);
	} else {
		//TRACE ("saving 0x%lx\n", (long)&stack);
	}
}
void 
FiberContext::load (void)
{
	longjmp (m_priv->m_context, 1);
	assert (false);
}

