/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-context.h"
#include <stdlib.h>
#include <assert.h>

struct FiberContext {
	uint32_t esp;
	uint32_t eip;
	uint8_t *stack;
};

struct FiberContext *
fiber_context_new_blank (void)
{
	struct FiberContext *self = (struct FiberContext *) malloc (sizeof (struct FiberContext));
	assert (self != NULL);
	self->stack = NULL;
	self->eip = 0;
	self->esp = 0;
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
	self->eip = (uint32_t) callback;
	self->esp = stack_end - 16;
	*((uint32_t *)(stack_end - 4)) = 0;
	*((uint32_t *)(stack_end - 8)) = 0;
	*((uint32_t *)(stack_end - 12)) = (uint32_t) data; /* arg 1*/
	*((uint32_t *)(stack_end - 16)) = 0; /* saved eip */

	return self;
}

void 
fiber_context_switch_to (struct FiberContext *from, 
			 struct FiberContext const *to)
{
	asm volatile("pushl %%ebp    \n\t"
		     "movl %%esp,%0  \n\t"
		     "movl %2,%%esp  \n\t"
		     "movl $1f,%1    \n\t"
		     "pushl %3       \n\t"
                     "ret            \n\t"
                     "1:             \n\t"
                     "popl %%ebp     \n\t"
		     : "=m" (from->esp),
		       "=m" (from->eip)
		     : "m" (to->esp),
		       "m" (to->eip));
}

void 
fiber_context_delete (struct FiberContext *context)
{
	if (context->stack != NULL) {
		free (context->stack);
		context->stack = NULL;
	}
	context->esp = 0;
	context->eip = 0;
	free (context);
}
