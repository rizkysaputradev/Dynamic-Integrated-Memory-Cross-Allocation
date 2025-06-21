#ifndef DIMCA_ADVANCED_ALLOCATOR_H
#define DIMCA_ADVANCED_ALLOCATOR_H

#include <stddef.h>

#define MAX_ADVANCED_LEVELS 5
#define BIN_COUNT 5
#define ALIGNMENT 16

typedef struct {
    int id;                 // Capsule ID
    int size;               // Requested size in bytes
    int preferred_level;    // Preferred memory level (1-based index)
    double time_cost;       // Allocation cost (output)
    void* address;          // Allocated address (output)
} DataCapsule;

void init_advanced_memory_levels(size_t* level_sizes, int num_levels);
void batch_cross_allocate(DataCapsule* capsules, size_t count);
void cross_free_advanced(void* ptr);
void shutdown_advanced_allocator();

#endif // DIMCA_ADVANCED_ALLOCATOR_H