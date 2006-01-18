/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-context.h"
#include <stdlib.h>
#include <assert.h>

struct FiberContext {
	uint8_t *stack;
};

struct FiberContext *
fiber_context_new_blank (void)
{
	struct FiberContext *self = (struct FiberContext *) malloc (sizeof (struct FiberContext));
	assert (self != NULL);
	self->stack = NULL;
	return self;
}

struct FiberContext *
fiber_context_new (void (*callback) (void *), 
		   void *data, 
		   uint32_t stack_size)
{
	uint8_t *stack = (uint8_t *)malloc (stack_size);
	uint32_t stack_end = ((uint32_t)stack) + stack_size;
	struct FiberContext *self = (struct FiberContext *) malloc (sizeof (struct FiberContext));
	assert (self != NULL);
	assert (stack != NULL);
	assert ((stack_end % 4) == 0);

	self->stack = stack;

	return self;
}

void 
fiber_context_switch_to (struct FiberContext *from, 
			 struct FiberContext const *to)
{
}

void 
fiber_context_delete (struct FiberContext *context)
{
	if (context->stack != NULL) {
		free (context->stack);
		context->stack = NULL;
	}
	free (context);
}
