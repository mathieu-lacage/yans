/* -*-	Mode:C; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */

#ifndef FIBER_CONTEXT_H
#define FIBER_CONTEXT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FiberContext;

struct FiberContext *fiber_context_new_blank (void);
struct FiberContext *fiber_context_new (void (*callback) (void *), 
					void *data, 
					uint32_t stack_size);
void fiber_context_switch_to (struct FiberContext *from, 
			      struct FiberContext const *to);
void fiber_context_delete (struct FiberContext *context);

#ifdef __cplusplus
}
#endif

#endif /* FIBER_CONTEXT_H */
