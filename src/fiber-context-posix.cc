/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-context.h"

#include <setjmp.h>

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
	setjmp (m_priv->m_context);
}
void 
FiberContext::load (void)
{
	longjmp (m_priv->m_context, 1);
}

