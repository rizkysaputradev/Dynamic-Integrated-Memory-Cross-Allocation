#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/dimca_basic.h"

#define DATA_COUNT 9
#define BATCHES 10

int main() {
    srand(time(NULL));
    double dimca_total = 0.0, normal_total = 0.0;

    printf("Running DIMCA vs Normal Allocation for %d batches...\n\n", BATCHES);

    for (int batch = 1; batch <= BATCHES; ++batch) {
        DataCapsule data[DATA_COUNT];
        for (int i = 0; i < DATA_COUNT; ++i) {
            data[i].id = i + 1;
            data[i].value = rand() % 100;
            data[i].level = (i < 5) ? 1 : (i < 8 ? 2 : 3); // levels: 5→3→1
        }

        // map: memory id → slot pointer
        MemorySlot* memory_map[10] = { NULL };

        MemoryField m1 = create_memory(3, 1, 1, memory_map);
        MemoryField m2 = create_memory(3, 2, 4, memory_map);
        MemoryField m3 = create_memory(3, 3, 7, memory_map);

        printf("=== Batch #%d ===\n", batch);
        printf("Running DIMCA Algorithm:\n");
        double dimca_time = run_dimca(data, DATA_COUNT, memory_map, 1, batch);
        dimca_total += dimca_time;

        reset_memory(&m1);
        reset_memory(&m2);
        reset_memory(&m3);

        printf("\nRunning Normal Allocation:\n");
        double normal_time = run_normal_allocator(data, DATA_COUNT, &m1, &m2, &m3, 1, batch);
        normal_total += normal_time;

        printf("\nBatch #%d Summary: DIMCA = %.2f, Normal = %.2f\n\n", batch, dimca_time, normal_time);

        // Cleanup
        free(m1.slots);
        free(m2.slots);
        free(m3.slots);
    }

    printf("=== FINAL AVERAGE RESULTS ===\n");
    printf("Average DIMCA Allocation Time:   %.5f seconds\n", dimca_total / BATCHES);
    printf("Average Normal Allocation Time:  %.5f seconds\n", normal_total / BATCHES);

    return 0;

}
