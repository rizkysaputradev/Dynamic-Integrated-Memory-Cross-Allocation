#ifndef DIMCA_VISUAL_ALLOCATOR_H
#define DIMCA_VISUAL_ALLOCATOR_H

#include <stddef.h>

#define MAX_VISUAL_LEVELS 5
#define BIN_COUNT 5
#define ALIGNMENT 16

typedef struct {
    int id;
    int size;
    int preferred_level;
    double time_cost;
    void* address;
} DataCapsule;

// Add this struct here:
typedef struct {
    size_t total_size;
    void* base;
    // Add more fields as needed for your memory level info
} AdvancedMemoryLevel;

// Extra logging function to emit state info per allocation
typedef void (*VisualizerLogger)(const char* message);

void init_visual_memory_levels(size_t* level_sizes, int num_levels, VisualizerLogger logger);
void batch_visual_allocate(DataCapsule* capsules, size_t count);
void visual_free(void* ptr);
void shutdown_visual_allocator();

#endif // DIMCA_VISUAL_ALLOCATOR_H
