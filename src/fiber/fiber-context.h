/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_CONTEXT_H
#define FIBER_CONTEXT_H

#include <stdint.h>

class FiberContextPrivate;

class FiberContext {
public:
	FiberContext ();
	~FiberContext ();

	/* This method should be called from the currently-saved
	 * context. It saves a new context.
	 */
	void save (void);
	/* restore the previously-saved context.
	 */
	void load (void);
private:
	FiberContextPrivate *m_priv;
};

#endif /* FIBER_CONTEXT_H */
