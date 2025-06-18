#ifndef DIMCA_BASIC_H
#define DIMCA_BASIC_H

typedef struct {
    int id;
    int value;
    int level;
} DataCapsule;

typedef struct {
    int id;
    int level;
    DataCapsule data;
    int occupied;
} MemorySlot;

typedef struct {
    MemorySlot* slots;
    int size;
    int level;
    int current_index;
} MemoryField;

MemoryField create_memory(int size, int level, int start_id, MemorySlot** memory_map);
void reset_memory(MemoryField* m);
double run_dimca(DataCapsule* data, int count, MemorySlot** memory_map, int verbose, int batch_num);
double run_normal_allocator(DataCapsule* data, int count, MemoryField* m1, MemoryField* m2, MemoryField* m3, int verbose, int batch_num);

#endif
