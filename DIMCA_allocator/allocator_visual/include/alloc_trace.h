#ifndef ALLOC_TRACE_H
#define ALLOC_TRACE_H

#include <stdint.h>
#include "dimca_visual_allocator.h"

typedef struct {
    uint32_t capsule_id;
    uint32_t memory_level;
    uint32_t memory_index;
    uint64_t timestamp;
    int32_t  penalty;
} AllocTraceEntry;

void write_allocation_trace(DataCapsule* capsules, int count, const char* filename, AdvancedMemoryLevel* mem_levels);


#endif // ALLOC_TRACE_H