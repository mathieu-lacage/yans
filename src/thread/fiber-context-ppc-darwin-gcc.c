/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#include "fiber-context.h"
#include <stdlib.h>
#include <assert.h>

struct FiberContext {
	uint8_t *stack;
	uint32_t r1;
};

void ppc_darwin_gcc_switch (uint32_t *from_r1, uint32_t to_r1);

struct FiberContext *
fiber_context_new_blank (void)
{
	struct FiberContext *self = (struct FiberContext *) malloc (sizeof (struct FiberContext));
	assert (self != NULL);
	self->stack = NULL;
	self->r1 = 0;
	return self;
}

struct FiberContext *
fiber_context_new (void (*callback) (void *), 
		   void *data, 
		   uint32_t stack_size)
{
	uint8_t *stack = (uint8_t *)malloc (stack_size);
	uint32_t *stack_end = (uint32_t *)(stack+stack_size+ (((uint32_t)stack) + stack_size + 4) % 4);
	uint8_t tmp;
	struct FiberContext *self = (struct FiberContext *) malloc (sizeof (struct FiberContext));
	assert (self != NULL);
	assert (stack != NULL);

	self->stack = stack;
	/* parameter area. */
	stack_end --;
	*stack_end = (uint32_t)data;
	for (tmp = 0; tmp < 7; tmp++) {
		stack_end--;
		*stack_end = 0;
	}
	/* reserved linkage area. */
	stack_end -= 3;
	/* linkage area */
	stack_end --; 
	*stack_end = (uint32_t)callback;
	stack_end -= 2;
	/* saved registers area. f14 to f31 */
	for (tmp = 0; tmp < (18*2); tmp++) {
		stack_end --;
		*stack_end = 0;
	}
	/* saved registers area. r3 */
	stack_end --;
	*stack_end = (uint32_t) data;
	/* saved registers area. r13 to r31 */
	for (tmp = 0; tmp < 19; tmp++) {
		stack_end --;
		*stack_end = 0;
	}
	/* reserved linkage area */
	stack_end -= 3;
	/* linkage area */
	stack_end -= 3;
	self->r1 = (uint32_t)stack_end;

	return self;
}

void 
fiber_context_switch_to (struct FiberContext *from, 
			 struct FiberContext const *to)
{
	ppc_darwin_gcc_switch (&from->r1, to->r1);
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
