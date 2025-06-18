#include <stdio.h>
#include <stdlib.h>
#include "../include/dimca_basic.h"

MemoryField create_memory(int size, int level, int start_id, MemorySlot** memory_map) {
    MemoryField m;
    m.slots = (MemorySlot*)malloc(sizeof(MemorySlot) * size);
    m.size = size;
    m.level = level;
    m.current_index = 0;
    for (int i = 0; i < size; i++) {
        int id = start_id + i;
        m.slots[i].id = id;
        m.slots[i].level = level;
        m.slots[i].occupied = 0;
        memory_map[id] = &m.slots[i];  // map memory ID to pointer
    }
    return m;
}

void reset_memory(MemoryField* m) {
    for (int i = 0; i < m->size; ++i) {
        m->slots[i].occupied = 0;
    }
    m->current_index = 0;
}

double calculate_time(int data_level, int memory_level, double n, double k) {
    return data_level == memory_level ? n : n + k;
}

void print_assignment(int batch, int iter, DataCapsule* d, MemorySlot* slot, double t) {
    printf("Batch %d | Iter %2d | Data ID: D%-2d (val=%3d, lvl=%d) --> Memory M%-2d (lvl=%d) | Time = %.2f\n",
        batch, iter, d->id, d->value, d->level, slot->id, slot->level, t);
}

double run_dimca(DataCapsule* data, int count, MemorySlot** memory_map, int verbose, int batch) {
    double total_time = 0.0;
    double n = 1.0, k = 0.5;

    int dimca_map[9] = {1, 2, 3, 6, 9, 4, 5, 8, 7}; // d1 → M1, ..., d9 → M7

    for (int i = 0; i < count; ++i) {
        DataCapsule* d = &data[i];
        int mem_id = dimca_map[i];
        MemorySlot* target = memory_map[mem_id];

        if (target) {
            target->data = *d;
            target->occupied = 1;
            double t = calculate_time(d->level, target->level, n, k);
            total_time += t;
            if (verbose) print_assignment(batch, i + 1, d, target, t);
        }
    }
    return total_time;
}

double run_normal_allocator(DataCapsule* data, int count, MemoryField* m1, MemoryField* m2, MemoryField* m3, int verbose, int batch) {
    double total_time = 0.0;
    double n = 1.0, k = 0.5;

    int iter = 1;
    int idx = 0;

    // Fill level 1 memory slots first with the first 3 data capsules
    for (int i = 0; i < m1->size && idx < count; ++i, ++idx) {
        DataCapsule* d = &data[idx];
        MemorySlot* slot = &m1->slots[i];
        slot->data = *d;
        slot->occupied = 1;
        double t = calculate_time(d->level, slot->level, n, k);
        total_time += t;
        if (verbose) print_assignment(batch, iter++, d, slot, t);
    }

    // Fill level 2 memory slots next with the next 3 data capsules
    for (int i = 0; i < m2->size && idx < count; ++i, ++idx) {
        DataCapsule* d = &data[idx];
        MemorySlot* slot = &m2->slots[i];
        slot->data = *d;
        slot->occupied = 1;
        double t = calculate_time(d->level, slot->level, n, k);
        total_time += t;
        if (verbose) print_assignment(batch, iter++, d, slot, t);
    }

    // Fill level 3 memory slots with remaining data capsules
    for (int i = 0; i < m3->size && idx < count; ++i, ++idx) {
        DataCapsule* d = &data[idx];
        MemorySlot* slot = &m3->slots[i];
        slot->data = *d;
        slot->occupied = 1;
        double t = calculate_time(d->level, slot->level, n, k);
        total_time += t;
        if (verbose) print_assignment(batch, iter++, d, slot, t);
    }

    return total_time;
}