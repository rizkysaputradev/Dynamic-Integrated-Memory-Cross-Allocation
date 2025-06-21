#include "dimca_advanced_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CAPSULE_COUNT 1000

int random_size() {
    return (rand() % 256) + 16; // Random size between 16 and 271 bytes
}

int random_level(int max_level) {
    return (rand() % max_level) + 1; // Level 1 to max_level
}

int main() {
    srand((unsigned int)time(NULL));

    size_t level_sizes[] = {
        64 * 1024,     // Level 1 (L1 Cache-style)
        128 * 1024,    // Level 2 (Main memory)
        256 * 1024     // Level 3 (Disk-simulated)
    };

    int num_levels = sizeof(level_sizes) / sizeof(level_sizes[0]);
    init_advanced_memory_levels(level_sizes, num_levels);

    DataCapsule capsules[CAPSULE_COUNT];
    for (int i = 0; i < CAPSULE_COUNT; i++) {
        capsules[i].id = i + 1;
        capsules[i].size = random_size();
        capsules[i].preferred_level = random_level(num_levels);
        capsules[i].time_cost = 0.0;
        capsules[i].address = NULL;
    }

    batch_cross_allocate(capsules, CAPSULE_COUNT);

    int success_count = 0;
    double total_cost = 0.0;
    for (int i = 0; i < CAPSULE_COUNT; i++) {
        if (capsules[i].address) {
            success_count++;
            total_cost += capsules[i].time_cost;
        }
    }

    printf("\n📊 DIMCA Advanced Benchmark Summary 📊\n");
    printf("Total Capsules: %d\n", CAPSULE_COUNT);
    printf("Successful Allocations: %d\n", success_count);
    printf("Success Rate: %.2f%%\n", (100.0 * success_count) / CAPSULE_COUNT);
    printf("Average Time Cost (for successful): %.2f\n", success_count ? total_cost / success_count : 0.0);

    for (int i = 0; i < CAPSULE_COUNT; i++) {
        if (capsules[i].address)
            cross_free_advanced(capsules[i].address);
    }

    shutdown_advanced_allocator();
    return 0;
}