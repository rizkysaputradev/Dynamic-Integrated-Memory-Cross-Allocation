#ifndef DIMCA_GENERAL_H
#define DIMCA_GENERAL_H

#include <pthread.h>

// Data capsule struct
typedef struct {
    int id;
    int value;
    int level;  // Data level
} DataCapsule;

// Memory slot struct
typedef struct {
    int id;
    int level;      // Memory level
    int occupied;   // 0 = free, 1 = occupied
    DataCapsule data;
} MemorySlot;

// Memory pool struct (flattened memory slots)
typedef struct {
    MemorySlot* all_slots;
    int total_slots;
    int levels_count; 
} MemoryPool;

// Create memory pool with given sizes and levels, returns initialized MemoryPool
MemoryPool create_memory_pool(int* sizes, int* levels, int count, MemorySlot** memory_map);

// Reset memory pool (mark all slots free)
void reset_memory_pool(MemoryPool* pool);

// Calculate time penalty for assigning data level to memory level
double calculate_time(int data_level, int memory_level, double n, double k);

// Concurrent allocator thread args
typedef struct {
    DataCapsule* data;
    int data_start;
    int data_end;
    MemoryPool* pool;
    int verbose;
    int batch;
    double partial_time;
    pthread_mutex_t* slot_mutexes;
    pthread_mutex_t* total_time_mutex;
} ThreadArgs;

// Run concurrent DIMCA allocator
double run_dimca_general_concurrent(DataCapsule* data, int data_count,
                                   MemoryPool* pool, int thread_count,
                                   int verbose, int batch);

#endif // DIMCA_GENERAL_H
