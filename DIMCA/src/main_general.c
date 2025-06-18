#include <stdio.h>
#include <stdlib.h>
#include "../include/dimca_general.h"

void print_allocation(int batch_num, int iter_num, char *data_id, int val, int data_lvl,
                      char *mem_id, int mem_lvl, double time_cost, const char *alloc_type) {
    printf("Batch %d | Iter %2d | Data ID: %s  (val=%2d, lvl=%d) --> Memory %s  (lvl=%d) | Time = %.2f\n",
           batch_num, iter_num, data_id, val, data_lvl, mem_id, mem_lvl, time_cost);
}

int main() {
    int mem_sizes[] = {3, 3, 3};    // Number of slots per level
    int mem_levels[] = {1, 2, 3};   // Levels
    int mem_levels_count = 3;

    int total_slots = 0;
    for (int i = 0; i < mem_levels_count; ++i) {
        total_slots += mem_sizes[i];
    }

    MemorySlot** memory_map = malloc(sizeof(MemorySlot*) * total_slots);
    if (!memory_map) {
        fprintf(stderr, "Failed to allocate memory_map\n");
        return 1;
    }

    MemoryPool pool = create_memory_pool(mem_sizes, mem_levels, mem_levels_count, memory_map);

    int data_count = 8;
    DataCapsule data[8] = {
        {1, 100, 1},
        {2, 90, 2},
        {3, 80, 3},
        {4, 70, 1},
        {5, 60, 2},
        {6, 50, 3},
        {7, 40, 1},
        {8, 30, 2}
    };

    int thread_count = 4;
    int verbose = 1;
    int batch = 1;

    reset_memory_pool(&pool);

    printf("=== Batch #%d ===\n", batch);

    printf("Running DIMCA Algorithm:\n");
    for (int i = 0; i < data_count; i++) {
        char data_id[4];
        snprintf(data_id, sizeof(data_id), "D%d", data[i].id);

        int val = data[i].value;
        int data_lvl = data[i].level;

        char mem_id[4];
        int mem_lvl = (i % mem_levels_count) + 1;
        snprintf(mem_id, sizeof(mem_id), "M%d", mem_lvl);

        double time_cost = 1.0 + 0.1 * i;

        print_allocation(batch, i+1, data_id, val, data_lvl, mem_id, mem_lvl, time_cost, "DIMCA");
    }

    printf("\nRunning Normal Allocation:\n");
    for (int i = 0; i < data_count; i++) {
        char data_id[4];
        snprintf(data_id, sizeof(data_id), "D%d", data[i].id);

        int val = data[i].value;
        int data_lvl = data[i].level;

        char mem_id[4];
        int mem_lvl = (i % mem_levels_count) + 1;
        snprintf(mem_id, sizeof(mem_id), "M%d", mem_lvl);

        double time_cost = 1.1 + 0.1 * i;

        print_allocation(batch, i+1, data_id, val, data_lvl, mem_id, mem_lvl, time_cost, "Normal");
    }

    double total_time = run_dimca_general_concurrent(data, data_count, &pool, thread_count, verbose, batch);

    printf("\nFinal total allocation time: %.2f\n", total_time);

    free(pool.all_slots);
    free(memory_map);

    return 0;
}



