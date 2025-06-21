#include "dimca_advanced_allocator.h"
#include <stdio.h>

int main() {
    // Step 1: Define memory levels
    size_t sizes[] = {64 * 1024, 128 * 1024, 256 * 1024}; // Level 1–3
    int num_levels = sizeof(sizes) / sizeof(sizes[0]);

    // Step 2: Initialize the DIMCA advanced allocator
    init_advanced_memory_levels(sizes, num_levels);

    // Step 3: Prepare a batch of data capsules
    DataCapsule capsules[] = {
        { .id = 1, .size = 40, .preferred_level = 1 },
        { .id = 2, .size = 70, .preferred_level = 1 },
        { .id = 3, .size = 120, .preferred_level = 3 },
        { .id = 4, .size = 200, .preferred_level = 2 },
        { .id = 5, .size = 300, .preferred_level = 1 }
    };
    size_t capsule_count = sizeof(capsules) / sizeof(capsules[0]);

    // Step 4: Perform the batch allocation
    batch_cross_allocate(capsules, capsule_count);

    // Step 5: Print the results
    for (size_t i = 0; i < capsule_count; i++) {
        printf("Capsule %d: requested %d bytes at Level %d → ptr = %p, cost = %.2f\n",
            capsules[i].id,
            capsules[i].size,
            capsules[i].preferred_level,
            capsules[i].address,
            capsules[i].time_cost
        );
    }

    // Step 6: Free the memory
    for (size_t i = 0; i < capsule_count; i++) {
        if (capsules[i].address) {
            cross_free_advanced(capsules[i].address);
            printf("Freed capsule %d\n", capsules[i].id);
        }
    }

    // Step 7: Shutdown the allocator
    shutdown_advanced_allocator();

    return 0;
}