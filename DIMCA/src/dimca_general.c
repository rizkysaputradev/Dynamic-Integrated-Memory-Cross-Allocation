#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/dimca_general.h"

// Create memory pool (similar to create_memory but for a pool)
MemoryPool create_memory_pool(int* sizes, int* levels, int count, MemorySlot** memory_map) {
    MemoryPool pool;
    int total_slots = 0;
    for (int i = 0; i < count; ++i) total_slots += sizes[i];

    pool.all_slots = (MemorySlot*)malloc(sizeof(MemorySlot) * total_slots);
    pool.total_slots = total_slots;
    pool.levels_count = count;

    int start_id = 0;
    int idx = 0;

    for (int i = 0; i < count; ++i) {
        int size = sizes[i];
        int level = levels[i];
        for (int j = 0; j < size; ++j) {
            int id = start_id + j;
            pool.all_slots[idx].id = id;
            pool.all_slots[idx].level = level;
            pool.all_slots[idx].occupied = 0;
            memory_map[id] = &pool.all_slots[idx];
            ++idx;
        }
        start_id += size;
    }

    return pool;
}

void reset_memory_pool(MemoryPool* pool) {
    for (int i = 0; i < pool->total_slots; ++i) {
        pool->all_slots[i].occupied = 0;
    }
}

double calculate_time(int data_level, int memory_level, double n, double k) {
    return data_level == memory_level ? n : n + k;
}

void* thread_allocate(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    DataCapsule* data = args->data;
    MemoryPool* pool = args->pool;
    double n = 1.0, k = 0.5;

    for (int i = args->data_start; i < args->data_end; ++i) {
        DataCapsule* d = &data[i];

        double best_time = -1;
        int best_idx = -1;

        for (int j = 0; j < pool->total_slots; ++j) {
            pthread_mutex_lock(&args->slot_mutexes[j]);
            if (!pool->all_slots[j].occupied) {
                double t = calculate_time(d->level, pool->all_slots[j].level, n, k);
                if (best_time < 0 || t < best_time) {
                    best_time = t;
                    best_idx = j;
                }
            }
            pthread_mutex_unlock(&args->slot_mutexes[j]);
        }

        if (best_idx >= 0) {
            pthread_mutex_lock(&args->slot_mutexes[best_idx]);
            if (!pool->all_slots[best_idx].occupied) {
                pool->all_slots[best_idx].data = *d;
                pool->all_slots[best_idx].occupied = 1;

                pthread_mutex_lock(args->total_time_mutex);
                args->partial_time += best_time;
                pthread_mutex_unlock(args->total_time_mutex);

                if (args->verbose) {
                    printf("Batch %d | Data %d assigned to Memory ID %d (lvl %d) | Time %.2f\n",
                        args->batch, d->id, pool->all_slots[best_idx].id, pool->all_slots[best_idx].level, best_time);
                }

                pthread_mutex_unlock(&args->slot_mutexes[best_idx]);
            } else {
                pthread_mutex_unlock(&args->slot_mutexes[best_idx]);
                if (args->verbose) {
                    printf("Batch %d | Data %d could not assign slot %d because it was occupied\n",
                        args->batch, d->id, pool->all_slots[best_idx].id);
                }
            }
        } else {
            if (args->verbose) {
                printf("Batch %d | Data %d could not be assigned (no free memory)\n", args->batch, d->id);
            }
        }
    }

    return NULL;
}

double run_dimca_general_concurrent(DataCapsule* data, int data_count,
                                   MemoryPool* pool, int thread_count,
                                   int verbose, int batch) {
    pthread_t* threads = malloc(sizeof(pthread_t) * thread_count);
    ThreadArgs* targs = malloc(sizeof(ThreadArgs) * thread_count);

    pthread_mutex_t* slot_mutexes = malloc(sizeof(pthread_mutex_t) * pool->total_slots);
    pthread_mutex_t total_time_mutex;
    pthread_mutex_init(&total_time_mutex, NULL);

    for (int i = 0; i < pool->total_slots; ++i) {
        pthread_mutex_init(&slot_mutexes[i], NULL);
    }

    int chunk_size = (data_count + thread_count - 1) / thread_count;
    double total_time = 0.0;

    for (int t = 0; t < thread_count; ++t) {
        int start = t * chunk_size;
        int end = (start + chunk_size) > data_count ? data_count : (start + chunk_size);

        targs[t].data = data;
        targs[t].data_start = start;
        targs[t].data_end = end;
        targs[t].pool = pool;
        targs[t].verbose = verbose;
        targs[t].batch = batch;
        targs[t].partial_time = 0.0;
        targs[t].slot_mutexes = slot_mutexes;
        targs[t].total_time_mutex = &total_time_mutex;

        pthread_create(&threads[t], NULL, thread_allocate, &targs[t]);
    }

    for (int t = 0; t < thread_count; ++t) {
        pthread_join(threads[t], NULL);
        total_time += targs[t].partial_time;
    }

    for (int i = 0; i < pool->total_slots; ++i) {
        pthread_mutex_destroy(&slot_mutexes[i]);
    }
    pthread_mutex_destroy(&total_time_mutex);
    free(slot_mutexes);
    free(threads);
    free(targs);

    if (verbose) {
        printf("Batch %d | Total allocation time: %.2f\n", batch, total_time);
    }
    return total_time;
}
