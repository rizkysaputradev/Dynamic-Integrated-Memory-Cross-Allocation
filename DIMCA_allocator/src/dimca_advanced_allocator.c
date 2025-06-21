#include "dimca_advanced_allocator.h"
#include "allocator_opt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)
#define ALIGNED(x) fast_align_up(x)

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
} AdvancedMemoryLevel;

static AdvancedMemoryLevel mem_levels[MAX_ADVANCED_LEVELS];
static int level_count = 0;
static double alpha = 0.5;

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

void init_advanced_memory_levels(size_t* sizes, int count) {
    if (count > MAX_ADVANCED_LEVELS) count = MAX_ADVANCED_LEVELS;
    level_count = count;
    for (int i = 0; i < count; i++) {
        mem_levels[i].base = mmap(NULL, sizes[i], PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (mem_levels[i].base == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }
        mem_levels[i].total_size = sizes[i];
        pthread_mutex_init(&mem_levels[i].lock, NULL);
        for (int j = 0; j < BIN_COUNT; j++)
            mem_levels[i].bins[j] = NULL;

        BlockHeader* block = (BlockHeader*)mem_levels[i].base;
        block->size = sizes[i] - BLOCK_HEADER_SIZE;
        block->free = 1;
        block->level = i + 1;
        block->next = block->prev = NULL;

        int bin = get_bin_index(block->size);
        mem_levels[i].bins[bin] = block;

        printf("Level %d initialized: base=%p, size=%zu → placed in bin[%d]\n", i + 1, mem_levels[i].base, sizes[i], bin);
    }
}

void batch_cross_allocate(DataCapsule* capsules, size_t count) {
    for (size_t c = 0; c < count; c++) {
        size_t aligned_size = ALIGNED(capsules[c].size);
        int bin_index = get_bin_index(aligned_size);

        int probe_order[MAX_ADVANCED_LEVELS];
        probe_order[0] = capsules[c].preferred_level - 1;
        int idx = 1;
        for (int i = 0; i < level_count; i++) {
            if (i != probe_order[0]) probe_order[idx++] = i;
        }

        for (int i = 0; i < level_count; i++) {
            int lvl = probe_order[i];
            AdvancedMemoryLevel* level = &mem_levels[lvl];
            pthread_mutex_lock(&level->lock);
            BlockHeader* curr = NULL;
            for (int b = bin_index; b < BIN_COUNT; b++) {
                curr = level->bins[b];
                while (curr) {
                    if (curr->free && curr->size >= aligned_size) {
                        curr->free = 0;
                        if (curr->size >= aligned_size + BLOCK_HEADER_SIZE + 16) {
                            BlockHeader* new_block = (BlockHeader*)((char*)curr + BLOCK_HEADER_SIZE + aligned_size);
                            new_block->size = curr->size - aligned_size - BLOCK_HEADER_SIZE;
                            new_block->free = 1;
                            new_block->level = curr->level;
                            new_block->next = curr->next;
                            new_block->prev = curr;
                            curr->next = new_block;
                            int new_bin = get_bin_index(new_block->size);
                            new_block->next = level->bins[new_bin];
                            level->bins[new_bin] = new_block;
                        }
                        pthread_mutex_unlock(&level->lock);
                        capsules[c].address = (void*)(curr + 1);
                        capsules[c].time_cost = calc_cost(capsules[c].preferred_level, lvl + 1);
                        goto next_capsule;
                    }
                    curr = curr->next;
                }
            }
            pthread_mutex_unlock(&level->lock);
        }
        capsules[c].address = NULL;
        capsules[c].time_cost = -1.0;
    next_capsule:;
    }
}

void cross_free_advanced(void* ptr) {
    if (!ptr) return;
    BlockHeader* block = (BlockHeader*)ptr - 1;
    AdvancedMemoryLevel* level = &mem_levels[block->level - 1];
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
    pthread_mutex_unlock(&level->lock);
}

void shutdown_advanced_allocator() {
    for (int i = 0; i < level_count; i++) {
        munmap(mem_levels[i].base, mem_levels[i].total_size);
        pthread_mutex_destroy(&mem_levels[i].lock);
    }
}