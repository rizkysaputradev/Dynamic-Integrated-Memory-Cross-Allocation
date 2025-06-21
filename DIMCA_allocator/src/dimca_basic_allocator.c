#include "dimca_basic_allocator.h"
#include <sys/mman.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ALIGNMENT 16
#define MAX_LEVELS 3
#define BIN_COUNT 5
#define BLOCK_HEADER_SIZE sizeof(BlockHeader)
#define ALIGNED(x) (((x + ALIGNMENT - 1) / ALIGNMENT) * ALIGNMENT)

typedef struct BlockHeader {
    size_t size;
    int free;
    int level;
    struct BlockHeader* next;
    struct BlockHeader* prev;
} BlockHeader;

typedef struct {
    pthread_mutex_t lock;
    BlockHeader* bins[BIN_COUNT];
    size_t total_size;
    void* base;
} MemoryLevel;

static MemoryLevel mem_levels[MAX_LEVELS];
static const size_t level_sizes[MAX_LEVELS] = {1024 * 64, 1024 * 128, 1024 * 256};
static const double alpha = 0.5;

static int get_bin_index(size_t size) {
    if (size <= 16) return 0;
    else if (size <= 32) return 1;
    else if (size <= 64) return 2;
    else if (size <= 128) return 3;
    return 4;
}

static double calc_cost(int preferred, int actual) {
    return 1.0 + alpha * abs(preferred - actual);
}

void init_memory_levels() {
    for (int i = 0; i < MAX_LEVELS; i++) {
        mem_levels[i].base = mmap(NULL, level_sizes[i], PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANON, -1, 0);
        if (mem_levels[i].base == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }
        mem_levels[i].total_size = level_sizes[i];
        pthread_mutex_init(&mem_levels[i].lock, NULL);
        for (int j = 0; j < BIN_COUNT; j++) {
            mem_levels[i].bins[j] = NULL;
        }
        
        printf("level %d initalized: base=%p, size=%zu\n", i + 1, mem_levels[i].base, level_sizes[i]);

        BlockHeader* block = (BlockHeader*)mem_levels[i].base;
        block->size = level_sizes[i] - BLOCK_HEADER_SIZE;
        block->free = 1;
        block->level = i + 1;
        block->next = block->prev = NULL;

        int bin = get_bin_index(block->size);
        mem_levels[i].bins[bin] = block;

        printf("→ Free block placed in bin[%d], block size = %zu (level %d)\n", bin, block->size, block->level);
    }
}

void* cross_malloc(size_t size, int preferred_level, double* time_cost_out) {
    size_t aligned_size = ALIGNED(size);
    int bin_index = get_bin_index(aligned_size);

    int probe_order[MAX_LEVELS] = {0};
    probe_order[0] = preferred_level - 1;
    int index = 1;
    for (int i = 0; i < MAX_LEVELS; i++) {
        if (i != preferred_level - 1) probe_order[index++] = i;
    }

    for (int i = 0; i < MAX_LEVELS; i++) {
        int lvl = probe_order[i];
        MemoryLevel* level = &mem_levels[lvl];

        pthread_mutex_lock(&level->lock);

        printf("🔍 Trying Level %d, bin %d+, aligned size %zu\n", lvl + 1, bin_index, aligned_size);

        BlockHeader* curr = NULL;
        for (int b = bin_index; b < BIN_COUNT; b++) {
            curr = level->bins[b];
            while (curr) {
                if (curr->free && curr->size >= aligned_size) {
                    // Allocate and split
                    curr->free = 0;

                    if (curr->size >= aligned_size + BLOCK_HEADER_SIZE + 16) {
                        BlockHeader* new_block = (BlockHeader*)((char*)curr + BLOCK_HEADER_SIZE + aligned_size);
                        new_block->size = curr->size - aligned_size - BLOCK_HEADER_SIZE;
                        new_block->free = 1;
                        new_block->level = curr->level;
                        new_block->next = curr->next;
                        new_block->prev = curr;

                        curr->next = new_block;
                        curr->size = aligned_size;

                        int new_bin = get_bin_index(new_block->size);
                        new_block->next = level->bins[new_bin];
                        level->bins[new_bin] = new_block;
                    }

                    pthread_mutex_unlock(&level->lock);
                    if (time_cost_out) *time_cost_out = calc_cost(preferred_level, lvl + 1);
                    return (void*)(curr + 1);
                }
                curr = curr->next;
            }
        }

        pthread_mutex_unlock(&level->lock);
    }

    if (time_cost_out) *time_cost_out = -1.0;
    return NULL;
}

void cross_free(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = (BlockHeader*)ptr - 1;
    MemoryLevel* level = &mem_levels[block->level - 1];

    pthread_mutex_lock(&level->lock);
    block->free = 1;

    if (block->next && block->next->free) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    if (block->prev && block->prev->free) {
        block->prev->size += BLOCK_HEADER_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }

    int bin = get_bin_index(block->size);
    block->next = level->bins[bin];
    level->bins[bin] = block;

    void* block_end = (void*)block + block->size + BLOCK_HEADER_SIZE;
    void* level_end = level->base + level->total_size;

    if (block_end == level_end && block->free && block->prev) {
        size_t shrinkable = block->size + BLOCK_HEADER_SIZE;
        if (munmap(block, shrinkable) == 0) {
            level->total_size -= shrinkable;
            block->prev->next = NULL;
        }
    }

    pthread_mutex_unlock(&level->lock);
}
