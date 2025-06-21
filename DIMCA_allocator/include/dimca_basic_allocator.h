#ifndef DIMCA_ALLOCATOR_H
#define DIMCA_ALLOCATOR_H

#include <stddef.h>

void init_memory_levels();
void* cross_malloc(size_t size, int preferred_level, double* time_cost_out);
void cross_free(void* ptr);

#endif // DIMCA_ALLOCATOR_H
