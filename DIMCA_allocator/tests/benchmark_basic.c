
#include "dimca_basic_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ALLOCS 10000
#define ALLOC_SIZE 64

static inline long long current_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main() {
    init_memory_levels();

    void* dimca_ptrs[NUM_ALLOCS];
    void* std_ptrs[NUM_ALLOCS];
    double total_cost = 0.0;

    // Benchmark cross_malloc
    long long start_dimca = current_time_ns();
    for (int i = 0; i < NUM_ALLOCS; i++) {
        double cost;
        dimca_ptrs[i] = cross_malloc(ALLOC_SIZE, 1, &cost);
        total_cost += cost;
    }
    long long end_dimca = current_time_ns();
    long long time_dimca = end_dimca - start_dimca;

    for (int i = 0; i < NUM_ALLOCS; i++) {
        cross_free(dimca_ptrs[i]);
    }

    // Benchmark malloc
    long long start_malloc = current_time_ns();
    for (int i = 0; i < NUM_ALLOCS; i++) {
        std_ptrs[i] = malloc(ALLOC_SIZE);
    }
    long long end_malloc = current_time_ns();
    long long time_malloc = end_malloc - start_malloc;

    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(std_ptrs[i]);
    }

    printf("DIMCA cross_malloc time: %lld ns\n", time_dimca);
    printf("Average DIMCA allocation cost: %.2f\n", total_cost / NUM_ALLOCS);
    printf("Standard malloc time:   %lld ns\n", time_malloc);

    return 0;
}
