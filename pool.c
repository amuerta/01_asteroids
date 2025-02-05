#ifndef __GAME_POOL_H
#define __GAME_POOL_H
#include "defs.h"

Entity* EntityPool_refer(EntityPool* p, Relptr ptr) {
	//static Entity* tmp = NULL; 
	//tmp = &(p.buffer[ptr]);
	return (p->state_buffer[ptr]) ? &(p->buffer[ptr]) : NULL;
}

Relptr 	EntityPool_reserve(EntityPool* p) {
	Relptr ptr = RELPTR_INVALID;
	if (p->free_count > 0) {
		ptr = p->free_buffer[p->free_count-1];
		p->free_count--;
	} else {
		ptr = p->count;
	}
	debug("Reserved id: %u\n",ptr);
	assert(ptr != RELPTR_INVALID && "Failed to reserve entity");
	
	p->state_buffer		[ptr] |= EntityState_allocated;
	assert(p->count < MAX_ENTITY_COUNT);

	p->count++;
	if (p->count > p->max_size) {
		p->max_size = p->count;
	}
	return ptr;
}

void	EntityPool_release(EntityPool* p, Relptr ptr) {
	if (p->count==0)
		return;

	assert(ptr < MAX_ENTITY_COUNT && "Attempt to access Out of Bounds");

	if (!(p->state_buffer[ptr] & EntityState_allocated))
		return;
	
	debug("Reserved id: %u\n",ptr);

	p->free_buffer[p->free_count] = ptr;
	p->free_count++;

	p->state_buffer[ptr] ^= EntityState_allocated;
	p->count--;
}

#endif //__GAME_POOL_H
